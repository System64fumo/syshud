#pragma once
#include <sigc++/connection.h>
#include <functional>

#define DEFAULT_FPS 60

typedef enum {
	PROPERTY_SIZE_REQUEST_WIDTH,
	PROPERTY_SIZE_REQUEST_HEIGHT,
	PROPERTY_SIZE_REQUEST_BOTH,
	PROPERTY_OPACITY,
	PROPERTY_SCALE_VALUE,
	PROPERTY_PROGRESS_VALUE
} property_type_t;

class property_animator {
	private:
		sigc::connection animation_connection;
		double start_value_1;
		double start_value_2;
		double target_value_1;
		double target_value_2;
		double current_value_1;
		double current_value_2;
		double animation_progress;
		double duration;
		unsigned int fps;
		std::function<void(double, double)> update_callback;
		std::function<void(double&, double&)> get_current_callback;

		double ease_out_cubic(double t);
		bool animate_step();

		template<typename widget_type_t>
		void setup_callbacks(widget_type_t* widget, property_type_t property);

	public:
		property_animator();
		~property_animator();

		void stop();

		template<typename widget_type_t>
		void animate_property(widget_type_t* widget, property_type_t property, 
							double target1, double duration_seconds, 
							double target2 = 0);
};