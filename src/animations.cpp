#include "animations.hpp"
#include <gtkmm/scale.h>
#include <gtkmm/progressbar.h>
#include <glibmm/main.h>
#include <cmath>

property_animator::property_animator() : 
	start_value_1(0), start_value_2(0), 
	target_value_1(0), target_value_2(0),
	current_value_1(0), current_value_2(0),
	animation_progress(0), duration(0.5), fps(DEFAULT_FPS) {
}

property_animator::~property_animator() {
	stop();
}

void property_animator::stop() {
	if (animation_connection.connected())
		animation_connection.disconnect();
}

double property_animator::ease_out_cubic(double t) {
	return 1.0 - std::pow(1.0 - t, 3.0);
}

bool property_animator::animate_step() {
	animation_progress += 1.0 / (duration * fps);
	
	if (animation_progress >= 1.0) {
		animation_progress = 1.0;
		current_value_1 = target_value_1;
		current_value_2 = target_value_2;
		update_callback(current_value_1, current_value_2);
		return false;
	}
	
	double eased_progress = ease_out_cubic(animation_progress);
	current_value_1 = start_value_1 + (target_value_1 - start_value_1) * eased_progress;
	current_value_2 = start_value_2 + (target_value_2 - start_value_2) * eased_progress;
	
	update_callback(current_value_1, current_value_2);
	return true;
}

template<typename widget_type_t>
void property_animator::animate_property(widget_type_t* widget, property_type_t property, 
									   double target1, double duration_seconds, 
									   double target2) {
	stop();
	
	duration = duration_seconds;
	target_value_1 = target1;
	target_value_2 = target2;
	
	setup_callbacks(widget, property);
	
	get_current_callback(start_value_1, start_value_2);
	current_value_1 = start_value_1;
	current_value_2 = start_value_2;
	
	animation_progress = 0.0;
	
	animation_connection = Glib::signal_timeout().connect(
		sigc::mem_fun(*this, &property_animator::animate_step),
		1000 / fps
	);
}

template<typename widget_type_t>
void property_animator::setup_callbacks(widget_type_t* widget, property_type_t property) {
	switch (property) {
		case PROPERTY_SIZE_REQUEST_WIDTH:
			update_callback = [widget](double w, double h) {
				int current_width, current_height;
				widget->get_size_request(current_width, current_height);
				widget->set_size_request(static_cast<int>(w), current_height);
			};
			get_current_callback = [widget](double& val1, double& val2) {
				int width, height;
				widget->get_size_request(width, height);
				val1 = static_cast<double>(width);
				val2 = 0.0;
			};
			break;
			
		case PROPERTY_SIZE_REQUEST_HEIGHT:
			update_callback = [widget](double w, double h) {
				int current_width, current_height;
				widget->get_size_request(current_width, current_height);
				widget->set_size_request(current_width, static_cast<int>(w));
			};
			get_current_callback = [widget](double& val1, double& val2) {
				int width, height;
				widget->get_size_request(width, height);
				val1 = static_cast<double>(height);
				val2 = 0.0;
			};
			break;
			
		case PROPERTY_SIZE_REQUEST_BOTH:
			update_callback = [widget](double w, double h) {
				widget->set_size_request(static_cast<int>(w), static_cast<int>(h));
			};
			get_current_callback = [widget](double& val1, double& val2) {
				int width, height;
				widget->get_size_request(width, height);
				val1 = static_cast<double>(width);
				val2 = static_cast<double>(height);
			};
			break;
			
		case PROPERTY_OPACITY:
			update_callback = [widget](double opacity, double unused) {
				widget->set_opacity(opacity);
			};
			get_current_callback = [widget](double& val1, double& val2) {
				val1 = widget->get_opacity();
				val2 = 0.0;
			};
			break;
			
		case PROPERTY_SCALE_VALUE:
			if (auto scale = dynamic_cast<Gtk::Scale*>(widget)) {
				update_callback = [scale](double value, double unused) {
					scale->set_value(value);
				};
				get_current_callback = [scale](double& val1, double& val2) {
					val1 = scale->get_value();
					val2 = 0.0;
				};
			}
			break;
			
		case PROPERTY_PROGRESS_VALUE:
			if (auto progress = dynamic_cast<Gtk::ProgressBar*>(widget)) {
				update_callback = [progress](double value, double unused) {
					progress->set_fraction(value);
				};
				get_current_callback = [progress](double& val1, double& val2) {
					val1 = progress->get_fraction();
					val2 = 0.0;
				};
			}
			break;
	}
}

template void property_animator::animate_property<Gtk::Widget>(Gtk::Widget*, property_type_t, double, double, double);
template void property_animator::animate_property<Gtk::Scale>(Gtk::Scale*, property_type_t, double, double, double);
template void property_animator::animate_property<Gtk::ProgressBar>(Gtk::ProgressBar*, property_type_t, double, double, double);

template void property_animator::setup_callbacks<Gtk::Widget>(Gtk::Widget*, property_type_t);
template void property_animator::setup_callbacks<Gtk::Scale>(Gtk::Scale*, property_type_t);
template void property_animator::setup_callbacks<Gtk::ProgressBar>(Gtk::ProgressBar*, property_type_t);