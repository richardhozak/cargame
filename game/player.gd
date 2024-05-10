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

@export var color: Color:
	set(value):
		color = value
		_update_color()

var spectate_camera: PhantomCamera3D = null

var replay := Replay.new()
var replay_input: int = -1
var playing_replay: bool = false

var initial_state := PackedByteArray()


func _update_color() -> void:
	var material := StandardMaterial3D.new()
	material.albedo_color = color
	$Node3D/Body/CSGBox3D.material = material

	var body := $Node3D/Body/body as MeshInstance3D
	var shader_material := body.get_active_material(0) as ShaderMaterial
	shader_material.set_shader_parameter("car_color", color)


func _enter_tree() -> void:
	set_multiplayer_authority(str(name).to_int())
	simulated.connect(_on_step_simulated)


func _on_step_simulated(step: CarPhysicsStep) -> void:
	const min_pitch := 1.0
	const max_pitch := 2.0
	const min_rpm := 1000
	const max_rpm := 6000

	var pitch := remap(step.rpm, min_rpm, max_rpm, min_pitch, max_pitch)
	%EngineSound.pitch_scale = pitch

	%CarSpeedLabel.text = "%.f" % absf(step.speed * 3.6)

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

	if step.simulated && replay_input == -1:
		if step.input.restart:
			replay = Replay.new()
		else:
			replay.add_input(step.input)


func set_spectate_camera(camera: PhantomCamera3D) -> void:
	spectate_camera = camera
	if spectate_camera:
		spectate_camera.set_follow_target(%Eye)
		spectate_camera.set_look_at_target(%LookAt)


func restart() -> void:
	if replay_input != -1:
		replay_input = 0
	var input := CarPhysicsInput.new()
	input.restart = true
	self.simulate(input)


func _physics_process(_delta: float) -> void:
	if paused:
		return

	if initial_state.size() > 0:
		load_state(initial_state)
		initial_state = PackedByteArray()

	if !is_multiplayer_authority() && !playing_replay:
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
	playing_replay = true
	replay_input = 0
	replay = to_play


func get_replay() -> Replay:
	return replay
