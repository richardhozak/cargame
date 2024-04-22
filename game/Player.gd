class_name Player extends CarPhysics

@export var paused := false

@export var player_name: String:
	get:
		return $Node3D/Body/Label3D.text
	set(value):
		$Node3D/Body/Label3D.text = value

@export var show_player_name: bool:
	get:
		return $Node3D/Body/Label3D.visible
	set(value):
		$Node3D/Body/Label3D.visible = value

var spectate_camera: PhantomCamera3D = null

var replay := Replay.new()
var replay_input: int = -1

var initial_state := PackedByteArray()


func _enter_tree() -> void:
	set_multiplayer_authority(str(name).to_int())
	simulated.connect(_on_step_simulated)


func _on_step_simulated(step: CarPhysicsStep) -> void:
	if step.just_finished:
		%FinishEye.global_transform = %Eye.global_transform
		%FinishLookAt.global_transform = %LookAt.global_transform
		if spectate_camera:
			spectate_camera.set_follow_target(%FinishEye)
			spectate_camera.set_look_at_target(%FinishLookAt)

	if step.input.restart:
		if spectate_camera:
			spectate_camera.set_follow_target(%Eye)
			spectate_camera.set_look_at_target(%LookAt)


func set_spectate_camera(camera: PhantomCamera3D) -> void:
	spectate_camera = camera
	if spectate_camera:
		spectate_camera.set_follow_target(%Eye)
		spectate_camera.set_look_at_target(%LookAt)


func restart() -> void:
	var input := CarPhysicsInput.new()
	input.restart = true
	self.simulate(input)


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
