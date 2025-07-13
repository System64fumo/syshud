#include "window.hpp"
#include <thread>
#include <QTimer>
#include <QFile>
#include <QBoxLayout>
#include <QWindow>
#include <QIcon>
#include <QPixmap>

#include <LayerShellQt/Shell>
#include <LayerShellQt/Window>

syshud::syshud(const std::map<std::string, std::map<std::string, std::string>>& cfg) : config_main(cfg) {
	setup_window();
	setup_layer_shell();

	// TODO: Enable/Disable listeners based on config
	listener_wireplumber = new syshud_wireplumber([this](double volume, bool output) {
		QMetaObject::invokeMethod(this, [this, volume, output]() {
			if (output)
				on_change('s', volume);
			else
				on_change('m', volume);
		});
	});

	listener_backlight = new syshud_backlight([this]() {
		QMetaObject::invokeMethod(this, [this]() {
			on_change('b', listener_backlight->get_brightness());
		});
	}, config_main["main"]["backlight-path"]);

	listener_keytoggles = new syshud_keytoggles([this]() {
		QMetaObject::invokeMethod(this, [this]() {
			on_change('k', 0);
		});
	}, config_main["main"]["keyboard-path"]);
}

void syshud::setup_window() {
	setWindowTitle("syshud");
	setObjectName("syshud");
	resize(std::stoi(config_main["main"]["width"]), std::stoi(config_main["main"]["height"]));

	const std::string& style_path = "/usr/share/sys64/hud/style.qss";
	const std::string& style_path_usr = std::string(getenv("HOME")) + "/.config/sys64/hud/style.qss";

	// Load base style
	if (std::filesystem::exists(style_path)) {
		loadStyleSheet(style_path);
	}
	// Load user style
	if (std::filesystem::exists(style_path_usr)) {
		loadStyleSheet(style_path_usr);
	}

	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_NoSystemBackground);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setAlignment(Qt::AlignCenter);

	QWidget* container = new QWidget(this);
	container->setObjectName("container");
	QBoxLayout* containerLayout = new QBoxLayout(QBoxLayout::LeftToRight, container);

	const bool vertical = config_main["main"]["orientation"] == "v";
	Qt::Alignment align = vertical ? Qt::AlignHCenter : Qt::AlignVCenter;

	icon_label = new QLabel();

	slider = new QSlider(Qt::Horizontal);
	slider->setRange(0, 100);
	if (vertical) {
		slider->setOrientation(Qt::Vertical);
		containerLayout->setDirection(QBoxLayout::TopToBottom);
		layout->setDirection(QBoxLayout::TopToBottom);
	}

	if (config_main["main"]["show-percentage"] == "true")
		label = new QLabel();

	hide_timer = new QTimer(this);
	hide_timer->setSingleShot(true);
	hide_timer->setInterval(std::stoi(config_main["main"]["timeout"]) * 1000);

	if (vertical) {
		if (config_main["main"]["show-percentage"] == "true")
			containerLayout->addWidget(label, 0 , Qt::AlignCenter);
		containerLayout->addWidget(slider, 1,  align);
		containerLayout->addWidget(icon_label, 0 , align);
	}
	else {
		containerLayout->addWidget(icon_label, 0 , align);
		containerLayout->addWidget(slider, 1,  align);
		if (config_main["main"]["show-percentage"] == "true")
			containerLayout->addWidget(label, 0 , Qt::AlignCenter);
	}

	// TODO: Make label have a fixed size
	if (config_main["main"]["show-percentage"] == "true") {
		label->setContentsMargins(0, 0, 0, 0);
		label->setEnabled(false);
	}

	layout->addWidget(container);

	QObject::connect(hide_timer, &QTimer::timeout, [this]() {
		hide();
	});

	QObject::connect(slider, &QSlider::valueChanged, [this](int value) {
		if (last_reason == 's')
			listener_wireplumber->set_volume(true, value);
		else if (last_reason == 'm')
			listener_wireplumber->set_volume(false, value);
		else if (last_reason == 'b')
			listener_backlight->set_brightness(value);
	});
}

void syshud::on_change(const char& reason, const int& value) {
	last_reason = reason;
	show();
	hide_timer->start();

	std::string icon_name;
	std::string label_text;
	std::map<int, std::string> value_levels = {
		{0, "muted"},
		{1, "low"},
		{2, "medium"},
		{3, "high"},
	};

	// Behold! The great if else hell
	// TODO: Consider not using unity or creating indie games
	if (reason == 's' || reason == 'm') {
		if (reason == 's') {
			if (listener_wireplumber->muted)
				icon_name = "audio-volume-muted";
			else
				icon_name = "audio-volume-" + value_levels[std::clamp(value / 34 + 1, 1, 3)];
		}
		else if (reason == 'm') {
			if (listener_wireplumber->muted)
				icon_name = "audio-input-microphone-muted";
			else
				icon_name = "audio-input-microphone-" + value_levels[std::clamp(value / 34 + 1, 1, 3)];
		}
		label_text = std::to_string(value) + "%";
		slider->show();
	}
	else if (reason == 'k') {
		if (listener_keytoggles->changed == 'c') {
			icon_name = listener_keytoggles->caps_lock ? "capslock-enabled" : "capslock-disabled";
			label_text = "Caps Lock";
		}
		else {
			icon_name = listener_keytoggles->num_lock ? "numlock-enabled" : "numlock-disabled";
			label_text = "Num Lock";
		}
		slider->hide();
	}
	else if (reason == 'b') {
		if (value == 0)
			icon_name = "display-brightness-off";
		else
			icon_name = "display-brightness-" + value_levels[value / 34 + 1];
		label_text = std::to_string(value) + "%";
		slider->show();
	}

	QIcon icon = QIcon::fromTheme(QString::fromStdString(icon_name + "-symbolic"));
	QPixmap pixmap = icon.pixmap(std::stoi(config_main["main"]["icon-size"]), std::stoi(config_main["main"]["icon-size"]));
	icon_label->setPixmap(pixmap);

	slider->blockSignals(true);
	slider->setValue(value);
	slider->blockSignals(false);

	if (config_main["main"]["show-percentage"] != "true")
		return;

	label->setText(QString::fromStdString(label_text));
}

void syshud::setup_layer_shell() {
	LayerShellQt::Shell::useLayerShell();
	createWinId();
	QWindow *window = windowHandle();

	LayerShellQt::Window *layer_shell_window = LayerShellQt::Window::get(window);
	layer_shell_window->setLayer(LayerShellQt::Window::Layer::LayerOverlay);
	layer_shell_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivity::KeyboardInteractivityNone);
	layer_shell_window->setScreenConfiguration(LayerShellQt::Window::ScreenFromCompositor);
	layer_shell_window->setExclusiveZone(0);
	layer_shell_window->setScope("syshud");

	// Anchors
	bool edge_top = (config_main["main"]["position"].find("top") != std::string::npos);
	bool edge_right = (config_main["main"]["position"].find("right") != std::string::npos);
	bool edge_bottom = (config_main["main"]["position"].find("bottom") != std::string::npos);
	bool edge_left = (config_main["main"]["position"].find("left") != std::string::npos);

	if ((edge_top && edge_bottom) || (edge_right && edge_left)) {
		std::fprintf(stderr, "Verry funny arguments you got there\n");
		std::fprintf(stderr, "Would be a shame if.. The program crashed right?\n");
		exit(1);
	}
	else if (!edge_top && !edge_right && !edge_bottom && !edge_left) {
		std::fprintf(stderr, "You sure you specified valid arguments?\n");
		std::fprintf(stderr, "Valid arguments: \"top right bottom left\"\n");
		exit(1);
	}

	LayerShellQt::Window::Anchors anchors;
	if (edge_top) anchors |= LayerShellQt::Window::AnchorTop;
	if (edge_right) anchors |= LayerShellQt::Window::AnchorRight;
	if (edge_bottom) anchors |= LayerShellQt::Window::AnchorBottom;
	if (edge_left) anchors |= LayerShellQt::Window::AnchorLeft;
	layer_shell_window->setAnchors(anchors);



	// Margins
	std::istringstream iss(config_main["main"]["margins"]);
	int margins[4] = {0};
	int count = 0;
	std::string margin_str;

	while (count < 4 && std::getline(iss, margin_str, ' '))
		margins[count++] = std::stoi(margin_str);

	layer_shell_window->setMargins({margins[0], margins[1], margins[2], margins[3]});
}

void syshud::loadStyleSheet(const std::string& filePath) {
	QFile styleFile(QString::fromStdString(filePath));
	if (styleFile.open(QFile::ReadOnly)) {
		QString styleSheet = QLatin1String(styleFile.readAll());
		this->setStyleSheet(styleSheet);
		styleFile.close();
	} else {
		std::fprintf(stderr, "Failed to load stylesheet: %s\n", filePath.c_str());
	}
}

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>& cfg) {
		return new syshud(cfg);
	}
}
