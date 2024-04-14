extends Node3D

const MAX_CLIENTS = 4
const PORT = 3535
const IP_ADDRESS = "127.0.0.1"

var create_server := false
var selected_track_uri := ""
var loaded_track: Node3D
var loaded_mesh: CarPhysicsTrackMesh
var player_name: String
var saved_state: PackedByteArray
var spectate_group := ButtonGroup.new()

const Player = preload("res://Player.tscn")
const PlayerSpectateItem = preload("res://PlayerSpectateItem.tscn")
var loaded_player: Player

var current_menu_state: MenuState = MenuState.MAIN_MENU

enum MenuState {
	NONE,
	MAIN_MENU,
	TRACK_SELECT_HOST,
	TRACK_SELECT_SINGLE_PLAYER,
	PAUSED,
}


class PeerState:
	var name: String = ""
	var level_loaded: bool = false
	var player: Player = null

	func _init(peer_name: String) -> void:
		name = peer_name


var peers := Dictionary()


func print_info(fn: String) -> void:
	prints(
		fn,
		"player",
		player_name,
		"remote",
		multiplayer.get_remote_sender_id(),
		"is_server",
		multiplayer.is_server()
	)


func _ready() -> void:
	spectate_group.allow_unpress = false
	spectate_group.pressed.connect(_on_spectate_pressed)
	player_name = RandomName.get_random_name()
	$PlayerName/PlayerNameContainer/PlayerName.text = player_name
	$DebugMenu/Menu/SaveReplayButton.pressed.connect(save_replay)
	$DebugMenu/Menu/LoadReplayButton.pressed.connect(load_replay)
	$DebugMenu/Menu/DisconnectButton.pressed.connect(disconnect_from_game)
	$DebugMenu/Menu/SaveStateButton.pressed.connect(save_state)
	$DebugMenu/Menu/LoadStateButton.pressed.connect(load_state)


func change_menu(menu_state: MenuState) -> void:
	match menu_state:
		MenuState.MAIN_MENU:
			if current_menu_state == MenuState.PAUSED:
				disconnect_from_game()
			create_server = false
			$PauseMenu.visible = false
			$SelectTrackMenu.visible = false
			$MainMenu.visible = true
		MenuState.TRACK_SELECT_HOST:
			create_server = true
			$PauseMenu.visible = false
			$SelectTrackMenu.visible = true
			$MainMenu.visible = false
		MenuState.TRACK_SELECT_SINGLE_PLAYER:
			create_server = false
			$PauseMenu.visible = false
			$SelectTrackMenu.visible = true
			$MainMenu.visible = false
		MenuState.PAUSED:
			if current_menu_state == MenuState.NONE:
				pause_children(true)
			$PauseMenu.visible = true
			$SelectTrackMenu.visible = false
			$MainMenu.visible = false
		MenuState.NONE:
			if current_menu_state == MenuState.PAUSED:
				pause_children(false)
			$PauseMenu.visible = false
			$SelectTrackMenu.visible = false
			$MainMenu.visible = false

	current_menu_state = menu_state


func pause_children(pause: bool):
	if !loaded_track:
		return

	if !create_server:
		for child in loaded_track.get_children():
			if child is Player:
				child.paused = pause


func _process(_delta: float) -> void:
	if Input.is_action_just_pressed("menu"):
		match current_menu_state:
			MenuState.MAIN_MENU:
				pass
			MenuState.TRACK_SELECT_HOST:
				change_menu(MenuState.MAIN_MENU)
			MenuState.TRACK_SELECT_SINGLE_PLAYER:
				change_menu(MenuState.MAIN_MENU)
			MenuState.PAUSED:
				change_menu(MenuState.NONE)
			MenuState.NONE:
				change_menu(MenuState.PAUSED)


func _on_spectate_pressed(button: BaseButton):
	var peer_id = button.get_meta("peer_id")
	if !peer_id:
		printerr("spected cannot find peer id")
		return

	var player := loaded_track.get_node_or_null(str(peer_id)) as Player
	if player:
		spectate(player)
	else:
		printerr("no player with id ", peer_id)


func spectate(player: Player) -> void:
	$PhantomCamera3D.set_follow_target(player.camera_eye)
	$PhantomCamera3D.set_look_at_target(player.camera_target)
	player.car_stats_changed.connect(display_car_stats, CONNECT_REFERENCE_COUNTED)
	player.countdown.connect(display_countdown, CONNECT_REFERENCE_COUNTED)
	player.simulation_step.connect(simulation_step, CONNECT_REFERENCE_COUNTED)


@rpc("any_peer", "call_local", "reliable")
func hello(peer_name: String) -> void:
	var remote_id := multiplayer.get_remote_sender_id()
	prints("got hello from", remote_id, peer_name)
	prints("sending load level")
	peers[remote_id] = PeerState.new(peer_name)
	load_level.rpc_id(remote_id, selected_track_uri)


@rpc("authority", "call_local", "reliable")
func load_level(level_name: String) -> void:
	prints("loading level", level_name, multiplayer.is_server())
	load_track(level_name)


@rpc("any_peer", "call_local", "reliable")
func level_loaded() -> void:
	var sender_id := multiplayer.get_remote_sender_id()
	var sender_state: PeerState = peers[sender_id]
	sender_state.level_loaded = true

	prints("level loaded, spawning player", sender_id, sender_state.name)

	# send currently loaded player spawn to every other player, including the currently loaded player
	for peer_id: int in peers:
		var peer_state: PeerState = peers[peer_id]
		if peer_state.level_loaded:
			spawn_player.rpc_id(peer_id, sender_id, sender_state.name, PackedByteArray())

	# send every spawned player to currently loaded player
	for peer_id: int in peers:
		if peer_id == sender_id:
			continue
		var peer_state: PeerState = peers[peer_id]
		if peer_state.player:
			spawn_player.rpc_id(sender_id, peer_id, peer_state.name, peer_state.player.save_state())


@rpc("authority", "call_local", "reliable")
func spawn_player(id: int, peer_name: String, initial_state: PackedByteArray) -> void:
	print_info("spawn_player")
	var is_local := multiplayer.get_unique_id() == id
	prints("spawn player", id, peer_name, "is server", multiplayer.is_server())
	var player := Player.instantiate()
	player.name = str(id)
	player.player_name = peer_name
	player.initial_state = initial_state
	player.set_disable_scale(true)

	var spectate_item := PlayerSpectateItem.instantiate()
	spectate_item.button_group = spectate_group
	spectate_item.player_name = peer_name
	$HUD/ScrollContainer/SpectateContainer.add_child(spectate_item)

	var spectate_button: BaseButton = spectate_group.get_buttons().back()
	spectate_button.set_meta("peer_id", id)

	loaded_track.add_child(player)

	# set camera for local player
	if is_local:
		loaded_player = player
		spectate_button.button_pressed = true

	# bind inputs simulated inputs, server distributes them to clients, clients just need to send them to server
	if multiplayer.is_server():
		player.input_simulated.connect(func(input): on_server_input_simulated(id, input))
	elif is_local:
		player.input_simulated.connect(on_local_input_simulated)

	if multiplayer.is_server():
		peers[id].player = player


@rpc("any_peer", "call_local", "reliable")
func player_spawned(id: int) -> void:
	pass


@rpc("any_peer", "call_remote", "reliable")
func input_simulated(input: CarPhysicsInput) -> void:
	var peer_id := multiplayer.get_remote_sender_id()
	var node := loaded_track.get_node_or_null(str(peer_id))
	if node:
		node.simulate_input(input)
	else:
		printerr("no player with id ", peer_id)


@rpc("authority", "call_local", "reliable")
func simulate_input(peer_id: int, input: CarPhysicsInput) -> void:
	var node := loaded_track.get_node_or_null(str(peer_id))
	if node:
		node.simulate_input(input)
	else:
		printerr("no player with id ", peer_id)


func on_local_input_simulated(input: CarPhysicsInput) -> void:
	# send input to server
	input_simulated.rpc_id(1, input)


func on_server_input_simulated(peer_id: int, input: CarPhysicsInput) -> void:
	for remote_peer_id in multiplayer.get_peers():
		if remote_peer_id != peer_id:
			var peer_state: PeerState = peers.get(remote_peer_id)
			if peer_state && peer_state.level_loaded:
				simulate_input.rpc_id(remote_peer_id, peer_id, input)


func peer_connected(id: int) -> void:
	# TODO: timeout peer if they do not send 'hello'
	return
	print("peer connected ", id, " ", multiplayer.is_server())
	var player := Player.instantiate()
	player.name = str(id)
	player.set_disable_scale(true)
	loaded_track.add_child(player)


func peer_disconnected(id: int) -> void:
	print("peer disconnected ", id, " ", multiplayer.is_server())
	var node := loaded_track.get_node_or_null(str(id))
	peers.erase(id)
	if node:
		node.queue_free()

	# TODO: send disconnect to other peers


func disconnect_from_game() -> void:
	for button in spectate_group.get_buttons():
		button.get_parent().queue_free()
	loaded_player = null
	$HUD.visible = false
	$PhantomCamera3D.set_follow_target(null)
	$PhantomCamera3D.set_look_at_target(null)
	if loaded_track:
		loaded_track.queue_free()
	multiplayer.multiplayer_peer = null


func host_game() -> void:
	# Create server.
	assert(multiplayer is SceneMultiplayer)
	multiplayer.server_relay = false
	multiplayer.allow_object_decoding = true

	if create_server:
		var peer := ENetMultiplayerPeer.new()
		peer.create_server(PORT, MAX_CLIENTS)
		multiplayer.multiplayer_peer = peer
		multiplayer.peer_connected.connect(peer_connected)
		multiplayer.peer_disconnected.connect(peer_disconnected)
	else:
		multiplayer.multiplayer_peer = OfflineMultiplayerPeer.new()

	hello.rpc_id(1, player_name)


func join_game() -> void:
	# Create client.
	multiplayer.allow_object_decoding = true
	var peer := ENetMultiplayerPeer.new()
	peer.create_client(IP_ADDRESS, PORT)
	multiplayer.multiplayer_peer = peer
	multiplayer.connected_to_server.connect(connected_to_server)
	multiplayer.server_disconnected.connect(server_disconnected)


func connected_to_server() -> void:
	hello.rpc_id(1, player_name)


func server_disconnected() -> void:
	disconnect_from_game()


func load_track(track_name: String) -> void:
	if loaded_track != null:
		remove_child(loaded_track)
		loaded_track = null
		loaded_mesh = null

	print("Load track")
	var document := GLTFDocument.new()
	var state := GLTFState.new()
	var error := document.append_from_file(track_name, state)
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
					if !physics_mesh.add_mesh(
						node.position,
						node.rotation,
						node.scale,
						vertices,
						indices,
						material.albedo_color
					):
						# TODO: this generates errors, try to make start and finish invisible different way
						mesh.clear()

		var scene_node := document.generate_scene(state)
		scene_node.scale = Vector3(10.0, 10.0, 10.0)
		scene_node.set_meta("mesh", physics_mesh)
		loaded_track = scene_node
		loaded_mesh = physics_mesh

		scene_node.ready.connect(track_ready)

		add_child(scene_node)

		#var player := Player.instantiate()
		#player.set_disable_scale(true)
		#player.ready.connect(player_ready.bind(player))
		#player.car_stats_changed.connect(display_car_stats)
		#player.countdown.connect(display_countdown)
		#loaded_player = player

		#scene_node.add_child(player)
	else:
		printerr("Couldn't load glTF scene (error code: %s)." % error_string(error))


func track_ready() -> void:
	level_loaded.rpc_id(1)
	$HUD.visible = true


func player_ready(player: Player) -> void:
	$PhantomCamera3D.set_follow_target(player.camera_eye)
	$PhantomCamera3D.set_look_at_target(player.camera_target)
	print("player ready", player.camera_target, $PhantomCamera3D.is_active())


func save_replay() -> void:
	var replay := loaded_player.get_replay()
	var result := ResourceSaver.save(replay, "user://replay.res", ResourceSaver.FLAG_COMPRESS)
	if result != OK:
		printerr("Could not save replay (error: %s)" % error_string(result))


func load_replay() -> void:
	var replay := ResourceLoader.load("user://replay.res")
	assert(replay != null)
	loaded_player.play_replay(replay)


func save_state() -> void:
	saved_state = loaded_player.save_state()


func load_state() -> void:
	if saved_state.size() > 0:
		loaded_player.load_state(saved_state)


func display_car_stats(speed: float, rpm: float, gear: int) -> void:
	$HUD/CarStats/Gear.text = "%d gear" % gear
	$HUD/CarStats/Speed.text = "%.f" % absf(speed * 3.6)
	$HUD/CarStats/Rpm.text = "%.f RPM" % rpm


func display_countdown(seconds: float) -> void:
	seconds = absf(floorf(seconds))
	if seconds > 0.0:
		$HUD/Countdown.visible = true
		$HUD/Countdown.text = String.num(seconds)
	else:
		$HUD/Countdown.text = "Start"
		await get_tree().create_timer(1.0).timeout
		$HUD/Countdown.visible = false


func human_time(step: int, full: bool = false) -> String:
	var is_negative := step < 0
	step = absi(step)
	var seconds := step as float / 60.0
	var minutes := (seconds / 60.0) as int
	var remaining_seconds = fmod(seconds, 60.0)
	var prefix := "-" if is_negative else " "
	var time = "%s%02d:%06.3f" % [prefix, minutes, remaining_seconds]
	if !full:
		time[time.length() - 1] = " "

	return time


func simulation_step(step: int) -> void:
	$HUD/Time.text = human_time(step)


func _on_main_menu_host() -> void:
	change_menu(MenuState.TRACK_SELECT_HOST)


func _on_main_menu_join() -> void:
	change_menu(MenuState.NONE)
	join_game()


func _on_main_menu_quit() -> void:
	get_tree().quit()


func _on_select_track_menu_selected(track_uri: String) -> void:
	change_menu(MenuState.NONE)
	selected_track_uri = track_uri
	host_game()


func _on_main_menu_single_player() -> void:
	change_menu(MenuState.TRACK_SELECT_SINGLE_PLAYER)


func _on_pause_menu_quit() -> void:
	get_tree().quit()


func _on_pause_menu_resume() -> void:
	change_menu(MenuState.NONE)


func _on_pause_menu_main_menu() -> void:
	change_menu(MenuState.MAIN_MENU)
