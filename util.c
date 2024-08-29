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