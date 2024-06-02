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

	$HUD.visible = is_spectated()


func _on_step_simulated(step: CarPhysicsStep) -> void:
	$Body.global_transform = step.transforms.body
	$Body/Wheel1.global_transform = step.transforms.wheel1
	$Body/Wheel2.global_transform = step.transforms.wheel2
	$Body/Wheel3.global_transform = step.transforms.wheel3
	$Body/Wheel4.global_transform = step.transforms.wheel4

	if $HUD.visible:
		_update_hud(step)

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
	$HUD.visible = true


func _on_follow_camera_became_inactive() -> void:
	if $FollowCamera.priority != 0:
		$FollowCamera.priority = 0
	show_player_name = true
	%EngineSound.volume_db = 0
	$HUD.visible = false


func spectate() -> void:
	$FollowCamera.priority = 1

	if !is_finished():
		if %LookAt.top_level:
			%LookAt.top_level = false
			%LookAt.transform = Transform3D.IDENTITY

		if %EyeSocket.top_level:
			%EyeSocket.top_level = false
			%EyeSocket.transform = Transform3D.IDENTITY

		if %AudioListener.top_level:
			%AudioListener.top_level = false
			%AudioListener.transform = Transform3D.IDENTITY

	%AudioListener.make_current()

	if name == str(multiplayer.get_unique_id()):
		$HUD/SpectatingLabel.text = ""
	else:
		$HUD/SpectatingLabel.text = "Spectating: %s" % player_name


func is_spectated() -> bool:
	return $FollowCamera.priority == 1


func restart() -> void:
	if driver is LocalPlayerInput || driver is ReplayPlayerInput:
		driver.restart()


func _physics_process(_delta: float) -> void:
	if paused:
		return

	Simulation.queue_simulate(self)


func _display_car_stats(speed: float, rpm: float, gear: int) -> void:
	$HUD/CarStats/Gear.text = "%d gear" % gear
	$HUD/CarStats/Speed.text = "%.f" % absf(speed * 3.6)
	$HUD/CarStats/Rpm.text = "%.f RPM" % rpm


func _display_countdown(step: int) -> void:
	if step < 0:
		var seconds := step / 100.0
		seconds = absf(floorf(seconds))
		$HUD/Countdown.visible = true
		$HUD/Countdown.text = String.num(seconds)
	elif step <= 60:
		$HUD/Countdown.text = "Start"
	else:
		$HUD/Countdown.visible = false


func _checkpoint_text(collected: int, available: int) -> String:
	if available > 0:
		return "%s / %s" % [collected, available]
	else:
		return ""


func _update_hud(step: CarPhysicsStep) -> void:
	if step.simulated:
		_display_car_stats(step.speed, step.rpm, step.gear)
		_display_countdown(step.step)
		$HUD/TrackStats/Time.text = Replays.human_time(step.step, step.just_finished)
		$HUD/TrackStats/Checkpoints.text = _checkpoint_text(
			step.collected_checkpoints, step.available_checkpoints
		)
