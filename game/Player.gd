class_name Player extends CarPhysics

@onready
var camera_target := $Node3D/Body/LookAt

@onready
var camera_eye := $Node3D/Eye

var replay := Replay.new()

var replay_input: int = -1

func _physics_process(delta: float) -> void:
	#print("replay input", replay_input)
	#print("replay", replay.get_count())
	if replay_input == -1:
		var input := CarPhysicsInput.new()
		input.up = Input.is_action_pressed("up")
		input.down = Input.is_action_pressed("down")
		input.left = Input.is_action_pressed("left")
		input.right = Input.is_action_pressed("right")
		input.brake = Input.is_action_pressed("brake")
		input.respawn = Input.is_action_pressed("respawn")
		input.restart = Input.is_action_pressed("restart")
		match self.simulate(input):
			SAVE:
				replay.add_input(input)
			NOOP:
				pass
			RESET:
				replay = Replay.new()
	else:
		if replay_input >= replay.get_count():
			self.simulate(CarPhysicsInput.new())
			return

		if replay_input == 0:
			var input := CarPhysicsInput.new()
			input.restart = true
			self.simulate(input)

		var input := replay.get_input(replay_input)
		self.simulate(input)
		replay_input += 1

func play_replay(to_play: Replay) -> void:
	replay_input = 0
	replay = to_play

func get_replay() -> Replay:
	return replay
