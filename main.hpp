#include <gtkmm.h>

inline Glib::RefPtr<Gtk::Application> app;
inline int timeout = 1;
inline int volume = 0;

class sysvol : public Gtk::Window {

	public:
		Gtk::Scale scale_volume;
		Glib::Dispatcher m_Dispatcher;
		void on_callback();
		sysvol();

	private:
		Gtk::Box box_layout;
		Gtk::Image image_volume;
		Gtk::Label label_volume;

		void InitLayout();
		void on_change();
};

inline sysvol* win;
