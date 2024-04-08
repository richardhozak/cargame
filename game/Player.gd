extends GDPhysics

@onready
var camera_target := $Node3D/Body/LookAt

@onready
var camera_eye := $Node3D/Eye

func _physics_process(delta: float) -> void:
	var input := PhysicsInput.new()
	input.up = Input.is_action_pressed("up")
	input.down = Input.is_action_pressed("down")
	input.left = Input.is_action_pressed("left")
	input.right = Input.is_action_pressed("right")
	input.brake = Input.is_action_pressed("brake")
	input.respawn = Input.is_action_pressed("respawn")
	input.restart = Input.is_action_pressed("restart")
	self.simulate(input)
