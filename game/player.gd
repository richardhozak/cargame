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
		%FinishAudioListener.global_transform = %AudioListener.global_transform
		%FinishAudioListener.make_current()
		$FollowCamera.look_at_mode = PhantomCamera3D.LookAtMode.NONE
		$FollowCamera.follow_mode = PhantomCamera3D.FollowMode.NONE

	if step.input.restart && $FollowCamera.priority == 1:
		spectate()


func spectate() -> void:
	var pcam_host: PhantomCameraHost = $FollowCamera.get_pcam_host_owner()
	var camera := pcam_host.get_active_pcam() as PhantomCamera3D
	if camera:
		var parent := camera.get_parent()
		if parent && parent is Player:
			parent.show_player_name = true
		camera.priority = 0

	show_player_name = false
	$FollowCamera.look_at_mode = PhantomCamera3D.LookAtMode.SIMPLE
	$FollowCamera.follow_mode = PhantomCamera3D.FollowMode.SIMPLE
	$FollowCamera.priority = 1
	%AudioListener.make_current()


func is_spectated() -> bool:
	return $FollowCamera.priority == 1


func restart() -> void:
	if driver is LocalPlayerInput || driver is ReplayPlayerInput:
		driver.restart()


func _physics_process(_delta: float) -> void:
	if paused:
		return

	for input in driver.get_next_inputs():
		self.simulate(input)
