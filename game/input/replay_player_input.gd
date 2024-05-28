class_name ReplayPlayerInput extends PlayerInput

var _replay: Replay
var _index: int = 0


func _init(replay: Replay) -> void:
	_replay = replay


func get_next_inputs() -> Array[CarPhysicsInput]:
	if _replay:
		# if replay is at the end, we return default input, so car gets simulated after it finishes
		if _index >= _replay.get_count():
			return [CarPhysicsInput.new()]

		var inputs: Array[CarPhysicsInput] = []

		if _index == 0:
			# if index is zero, we send restart input alongside
			# first input to reset the player's car in case it is being rerun
			var restart_input := CarPhysicsInput.new()
			restart_input.restart = true
			inputs.push_back(restart_input)

		inputs.push_back(_replay.get_input(_index))
		_index += 1
		return inputs
	else:
		return []


func get_replay() -> Replay:
	return _replay


func restart() -> void:
	_index = 0
