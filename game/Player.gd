class_name Player extends CarPhysics

@onready var camera_target := $Node3D/Body/LookAt
@onready var camera_eye := $Node3D/Eye

@export var paused := false

@export var player_name: String:
	get:
		return $Node3D/Body/Label3D.text
	set(value):
		$Node3D/Body/Label3D.text = value

var replay := Replay.new()
var replay_input: int = -1

var initial_state := PackedByteArray()

signal input_simulated(input: CarPhysicsInput)


func _enter_tree() -> void:
	set_multiplayer_authority(str(name).to_int())
	simulated.connect(func(step): if step.input_simulated: input_simulated.emit(step.input))


func _physics_process(_delta: float) -> void:
	if paused:
		return

	if initial_state.size() > 0:
		load_state(initial_state)
		initial_state = PackedByteArray()

	if !is_multiplayer_authority():
		if replay.get_count() == 0:
			return

		if replay_input < replay.get_count():
			var input := replay.get_input(replay_input)
			self.simulate(input)
			replay_input += 1
			return

		return

	if replay_input == -1:
		var input := CarPhysicsInput.new()
		input.up = Input.is_action_pressed("up")
		input.down = Input.is_action_pressed("down")
		input.left = Input.is_action_pressed("left")
		input.right = Input.is_action_pressed("right")
		input.brake = Input.is_action_pressed("brake")
		input.respawn = Input.is_action_pressed("respawn")
		input.restart = Input.is_action_pressed("restart")
		self.simulate(input)
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


func simulate_input(input: CarPhysicsInput) -> void:
	if replay_input == -1:
		replay_input = 0

	replay.add_input(input)


func play_replay(to_play: Replay) -> void:
	replay_input = 0
	replay = to_play


func get_replay() -> Replay:
	return replay
