bool almost_equals(float a, float b, float epsilon) {
	return fabs(a - b) <= epsilon;
}

bool animate_to_target_f32(float* value, float target, float delta_time, float rate) {
	*value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_time));
	if (almost_equals(*value, target, 0.001f)) {
		*value = target;
		return true; // finished animation
	}
	return false;
}

bool animate_to_target_v2(Vector2* value, Vector2 target, float delta_time, float rate) {
	bool result_x = animate_to_target_f32(&(value->x), target.x, delta_time, rate);
	bool result_y = animate_to_target_f32(&(value->y), target.y, delta_time, rate);
	return result_x && result_y;
}

Vector2 screen_to_world(Vector2 screen) {
	Matrix4 proj = draw_frame.projection;
	Matrix4 view = draw_frame.camera_xform;
	float window_w = window.width;
	float window_h = window.height;

	// Normalize the mouse coordinates
	float ndc_x = (screen.x / (window_w * 0.5f)) - 1.0f;
	float ndc_y = (screen.y / (window_h * 0.5f)) - 1.0f;

	// Transform to world coordinates
	Vector4 world_pos = v4(ndc_x, ndc_y, 0, 1);
	world_pos = m4_transform(m4_inverse(proj), world_pos);
	world_pos = m4_transform(view, world_pos);
	
	return world_pos.xy;
}

Vector2 mouse_world_pos() {
	return screen_to_world(v2(input_frame.mouse_x, input_frame.mouse_y));
}