class_name RemotePlayerInput extends PlayerInput

var _inputs: Array[CarPhysicsInput] = []
var _replay_input = -1


func get_next_inputs() -> Array[CarPhysicsInput]:
	if _replay_input == -1:
		return []

	if _replay_input >= _inputs.size():
		return []

	var input := _inputs[_replay_input]
	_replay_input += 1
	return [input]


func queue_input(input: CarPhysicsInput) -> void:
	if _replay_input == -1:
		_replay_input = 0
	_inputs.push_back(input)
