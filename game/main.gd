extends Node3D

var loaded_track: Node3D
var loaded_mesh: CarPhysicsTrackMesh

const Player = preload("res://Player.tscn")
var loaded_player: Player

func _ready() -> void:
	$HUD/Menu/LoadTrackButton.pressed.connect(load_track)
	$HUD/Menu/SaveReplayButton.pressed.connect(save_replay)
	$HUD/Menu/LoadReplayButton.pressed.connect(load_replay)

func load_track() -> void:
	if loaded_track != null:
		remove_child(loaded_track)
		loaded_track = null
		loaded_mesh = null
	
	print("Load track")
	var document := GLTFDocument.new()
	var state := GLTFState.new()
	var error := document.append_from_file("res://track_straight.glb", state)
	if error == OK:
		var physics_mesh := CarPhysicsTrackMesh.new()
		for node in state.nodes:
			if node.mesh != -1:
				var gltf_mesh := state.meshes[node.mesh]
				var mesh := gltf_mesh.mesh
				for surface_idx in mesh.get_surface_count():
					var material := mesh.get_surface_material(surface_idx)
					#print(node.position, ", ", node.rotation, ", ", node.scale, ", ", surface_idx, ", ", material.albedo_color)
					var arrays := mesh.get_surface_arrays(surface_idx)
					var vertices: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
					var indices: PackedInt32Array = arrays[Mesh.ARRAY_INDEX]
					if !physics_mesh.add_mesh(node.position, node.rotation, node.scale, vertices, indices, material.albedo_color):
						# TODO: this generates errors, try to make start and finish invisible different way
						mesh.clear()

		var scene_node := document.generate_scene(state)
		scene_node.scale = Vector3(10.0, 10.0, 10.0)
		scene_node.set_meta("mesh", physics_mesh)
		
		var player := Player.instantiate()
		player.set_disable_scale(true)
		player.ready.connect(player_ready.bind(player))
		player.car_stats_changed.connect(display_car_stats)
		loaded_player = player
		
		loaded_track = scene_node
		loaded_mesh = physics_mesh
		add_child(scene_node)
		scene_node.add_child(player)

	else:
		printerr("Couldn't load glTF scene (error code: %s)." % error_string(error))

func player_ready(player: Player) -> void:
	$PhantomCamera3D.set_follow_target(player.camera_eye)
	$PhantomCamera3D.set_look_at_target(player.camera_target)
	print("player ready", player.camera_target, $PhantomCamera3D.is_active())
	
func save_replay():
	var replay = loaded_player.get_replay()
	var result := ResourceSaver.save(replay, "user://replay.res", ResourceSaver.FLAG_COMPRESS)
	if result != OK:
		printerr("Could not save replay (error: %s)" % error_string(result))

func load_replay():
	var replay = ResourceLoader.load("user://replay.res")
	assert(replay != null)
	loaded_player.play_replay(replay)
	
func display_car_stats(speed: float, rpm: float, gear: int) -> void:
	$HUD/CarStats/Gear.text = "%d gear" % gear
	$HUD/CarStats/Speed.text = "%.f" % absf(speed * 3.6)
	$HUD/CarStats/Rpm.text = "%.f RPM" % rpm
