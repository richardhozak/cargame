class_name LocalPlayerInput extends PlayerInput

var _replay := Replay.new()
var _restart := false


func get_next_inputs() -> Array[CarPhysicsInput]:
	var input := CarPhysicsInput.new()
	input.up = Input.is_action_pressed("up")
	input.down = Input.is_action_pressed("down")
	input.left = Input.is_action_pressed("left")
	input.right = Input.is_action_pressed("right")
	input.brake = Input.is_action_pressed("brake")
	input.respawn = Input.is_action_pressed("respawn")
	input.restart = Input.is_action_pressed("restart") || _restart
	_restart = false

	if input.restart:
		_replay = Replay.new()
	else:
		_replay.add_input(input)

	return [input]


func get_replay() -> Replay:
	return _replay


func restart() -> void:
	_restart = true
