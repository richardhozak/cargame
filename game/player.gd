class_name Player extends CarPhysics

@export var paused := false

@export var player_name: String:
	get:
		return $Body/PlayerNameLabel.text
	set(value):
		$Body/PlayerNameLabel.text = value

@export var show_player_name: bool:
	get:
		return $Body/PlayerNameLabel.visible
	set(value):
		$Body/PlayerNameLabel.visible = value

@export var color: Color:
	set(value):
		color = value
		_update_color()

var driver: PlayerInput
var initial_state := PackedByteArray()


func _update_color() -> void:
	var material := StandardMaterial3D.new()
	material.albedo_color = color
	$Body/CSGBox3D.material = material

	var body := $Body/body as MeshInstance3D
	var shader_material := body.get_active_material(0) as ShaderMaterial
	shader_material.set_shader_parameter("car_color", color)


func _enter_tree() -> void:
	set_multiplayer_authority(str(name).to_int())
	simulated.connect(_on_step_simulated)


func _ready() -> void:
	await ready
	if initial_state.size() > 0:
		load_state(initial_state)
		initial_state = PackedByteArray()


func _on_step_simulated(step: CarPhysicsStep) -> void:
	$Body.global_transform = step.transforms.body
	$Body/Wheel1.global_transform = step.transforms.wheel1
	$Body/Wheel2.global_transform = step.transforms.wheel2
	$Body/Wheel3.global_transform = step.transforms.wheel3
	$Body/Wheel4.global_transform = step.transforms.wheel4

	const min_pitch := 1.0
	const max_pitch := 2.5
	const min_rpm := 1000
	const max_rpm := 6000

	var pitch := remap(step.rpm, min_rpm, max_rpm, min_pitch, max_pitch)
	%EngineSound.pitch_scale = pitch

	%CarSpeedLabel.text = "%.f" % absf(step.speed * 3.6)

	if step.just_finished:
		%LookAt.top_level = true
		%EyeSocket.top_level = true
		%AudioListener.top_level = true

	if step.input.restart && $FollowCamera.priority == 1:
		spectate()


func _on_follow_camera_became_active() -> void:
	show_player_name = false
	%EngineSound.volume_db = -30


func _on_follow_camera_became_inactive() -> void:
	if $FollowCamera.priority != 0:
		$FollowCamera.priority = 0
	show_player_name = true
	%EngineSound.volume_db = 0


func spectate() -> void:
	$FollowCamera.priority = 1

	if !is_finished():
		if %LookAt.top_level:
			%LookAt.top_level = false
			%LookAt.position = Vector3.ZERO

		if %EyeSocket.top_level:
			%EyeSocket.top_level = false
			%EyeSocket.position = Vector3.ZERO

		if %AudioListener.top_level:
			%AudioListener.top_level = false
			%AudioListener.position = Vector3.ZERO

	%AudioListener.make_current()


func is_spectated() -> bool:
	return $FollowCamera.priority == 1


func restart() -> void:
	if driver is LocalPlayerInput || driver is ReplayPlayerInput:
		driver.restart()


func _physics_process(_delta: float) -> void:
	if paused:
		return

	Simulation.queue_simulate(self)
