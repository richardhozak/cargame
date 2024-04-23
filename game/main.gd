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
	FINISHED,
	LOAD_REPLAY,
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
	$DebugMenu/Menu/SaveStateButton.pressed.connect(save_state)
	$DebugMenu/Menu/LoadStateButton.pressed.connect(load_state)
	change_menu(MenuState.MAIN_MENU)


func change_menu(menu_state: MenuState) -> void:
	var current_menu := get_node_or_null("menu")
	if current_menu:
		# change the name so it does not collide with newly spawned menu
		current_menu.name = "old_menu"
		current_menu.queue_free()

	match menu_state:
		MenuState.MAIN_MENU:
			match current_menu_state:
				MenuState.PAUSED, MenuState.FINISHED:
					prints("Disconnect from game")
					disconnect_from_game()
			create_server = false

			var menu := preload("res://menus/MainMenu.tscn").instantiate()
			menu.name = "menu"
			menu.single_player.connect(_on_main_menu_single_player)
			menu.host.connect(_on_main_menu_host)
			menu.join.connect(_on_main_menu_join)
			menu.quit.connect(_on_main_menu_quit)
			add_child(menu)
		MenuState.TRACK_SELECT_HOST:
			create_server = true
			var menu := preload("res://menus/SelectTrackMenu.tscn").instantiate()
			menu.name = "menu"
			menu.selected.connect(_on_select_track_menu_selected)
			add_child(menu)
		MenuState.TRACK_SELECT_SINGLE_PLAYER:
			create_server = false
			var menu := preload("res://menus/SelectTrackMenu.tscn").instantiate()
			menu.name = "menu"
			menu.selected.connect(_on_select_track_menu_selected)
			add_child(menu)
		MenuState.PAUSED:
			if current_menu_state == MenuState.NONE:
				pause_children(true)

			var menu := preload("res://menus/PauseMenu.tscn").instantiate()
			menu.name = "menu"
			menu.resume.connect(_on_pause_menu_resume)
			menu.load_replay.connect(_on_pause_menu_load_replay)
			menu.main_menu.connect(_on_pause_menu_main_menu)
			menu.quit.connect(_on_pause_menu_quit)
			add_child(menu)
		MenuState.FINISHED:
			var menu := preload("res://menus/FinishMenu.tscn").instantiate()
			menu.name = "menu"
			menu.restart.connect(_on_finish_menu_restart)
			menu.save_replay.connect(_on_finish_menu_save_replay)
			menu.main_menu.connect(_on_pause_menu_main_menu)
			menu.quit.connect(_on_pause_menu_quit)
			add_child(menu)
		MenuState.NONE:
			if current_menu_state == MenuState.PAUSED:
				pause_children(false)
		MenuState.LOAD_REPLAY:
			var menu := preload("res://menus/LoadReplayMenu.tscn").instantiate()
			menu.name = "menu"
			menu.replay_toggled.connect(_on_replay_menu_replay_toggled)
			add_child(menu)

	current_menu_state = menu_state


func pause_children(pause: bool) -> void:
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
			MenuState.FINISHED:
				pass
			MenuState.NONE:
				change_menu(MenuState.PAUSED)
			MenuState.LOAD_REPLAY:
				change_menu(MenuState.PAUSED)


func _on_spectate_pressed(button: BaseButton) -> void:
	var peer_id = button.get_meta("peer_id")
	if !peer_id:
		printerr("spected cannot find peer id")
		return

	spectate(peer_id)


func spectate(peer_id: int) -> void:
	if !loaded_track:
		printerr("cannot spectate peer %d, track is not loaded" % peer_id)
		return

	var player := loaded_track.get_node_or_null(str(peer_id)) as Player
	if !player:
		printerr("cannot find peer %d in loaded track" % peer_id)
		return

	var spectated_peer_id := $PhantomCamera3D.get_meta("spectated_peer_id", 0) as int
	if spectated_peer_id != 0:
		var spectated_player := loaded_track.get_node_or_null(str(spectated_peer_id)) as Player
		if spectated_player:
			spectated_player.set_spectate_camera(null)
			spectated_player.show_player_name = true
			spectated_player.simulated.disconnect(_on_spectated_player_simulated)

	player.show_player_name = false

	if peer_id == multiplayer.get_unique_id():
		$HUD/SpectatingLabel.text = ""
	else:
		$HUD/SpectatingLabel.text = "Spectating: %s" % player.player_name

	player.set_spectate_camera($PhantomCamera3D)
	$PhantomCamera3D.set_meta("spectated_peer_id", peer_id)
	player.simulated.connect(_on_spectated_player_simulated)


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
	var is_replay := id < 0
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

	if is_replay:
		return

	# set camera for local player
	if is_local:
		loaded_player = player
		spectate_button.button_pressed = true

	# bind inputs simulated inputs, server distributes them to clients, clients just need to send them to server
	if multiplayer.is_server():
		player.simulated.connect(func(step): on_server_input_simulated(id, step.input))

	if is_local:
		player.simulated.connect(func(step): on_local_step_simulated(step))

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


func on_local_step_simulated(step: CarPhysicsStep) -> void:
	if !multiplayer.is_server():
		# send input to server
		input_simulated.rpc_id(1, step.input)

	if step.just_finished:
		change_menu(MenuState.FINISHED)
		$menu.set_time(human_time(step.step, step.just_finished))

	if step.input.restart:
		if current_menu_state == MenuState.FINISHED:
			change_menu(MenuState.NONE)


func on_server_input_simulated(peer_id: int, input: CarPhysicsInput) -> void:
	for remote_peer_id in multiplayer.get_peers():
		if remote_peer_id != peer_id:
			var peer_state: PeerState = peers.get(remote_peer_id)
			if peer_state && peer_state.level_loaded:
				simulate_input.rpc_id(remote_peer_id, peer_id, input)


@rpc("authority", "call_local", "reliable")
func despawn_player(id: int) -> void:
	peers.erase(id)

	for button in spectate_group.get_buttons():
		if button.get_meta("peer_id") == id:
			button.get_parent().queue_free()

	var node := loaded_track.get_node_or_null(str(id))
	if node:
		if $PhantomCamera3D.get_meta("spectated_peer_id") == id:
			$PhantomCamera3D.set_follow_target(null)
			$PhantomCamera3D.set_look_at_target(null)

			var local_peer_id := str(loaded_player.name).to_int()
			spectate(local_peer_id)

		node.queue_free()


func peer_connected(id: int) -> void:
	# TODO: timeout peer if they do not send 'hello'
	return
	print("peer connected ", id, " ", multiplayer.is_server())
	var player := Player.instantiate()
	player.name = str(id)
	player.set_disable_scale(true)
	loaded_track.add_child(player)


func peer_disconnected(id: int) -> void:
	despawn_player.rpc_id(1, id)
	for peer_id in multiplayer.get_peers():
		if peer_id != id:
			despawn_player.rpc_id(peer_id, id)


func disconnect_from_game() -> void:
	for button in spectate_group.get_buttons():
		button.get_parent().queue_free()
	loaded_player = null
	$HUD.visible = false
	$PhantomCamera3D.set_follow_target(null)
	$PhantomCamera3D.set_look_at_target(null)
	if loaded_track:
		loaded_track.queue_free()
		loaded_track = null
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

	for p in peer.host.get_peers():
		p.set_timeout(500, 1000, 2000)

	multiplayer.multiplayer_peer = peer
	multiplayer.connected_to_server.connect(connected_to_server)
	multiplayer.connection_failed.connect(connection_failed)
	multiplayer.server_disconnected.connect(server_disconnected)


func connected_to_server() -> void:
	print("connected to server")
	var peer := multiplayer.multiplayer_peer
	if peer is ENetMultiplayerPeer:
		for p: ENetPacketPeer in peer.host.get_peers():
			p.set_timeout(1000, 2000, 4000)

	hello.rpc_id(1, player_name)


func server_disconnected() -> void:
	print("server disconnected")
	disconnect_from_game()
	change_menu(MenuState.MAIN_MENU)


func connection_failed() -> void:
	print("connection failed")
	change_menu(MenuState.MAIN_MENU)


func override_materials(node: Node):
	if node is MeshInstance3D:
		var mesh: Mesh = node.mesh
		for index in mesh.get_surface_count():
			var material := mesh.surface_get_material(index)
			if material is BaseMaterial3D:
				prints(material.albedo_color)
				if material.albedo_color.is_equal_approx(Color(1, 0, 0, 1)):
					material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
					material.albedo_color = Color(1, 0, 0, 0.5)
				elif material.albedo_color.is_equal_approx(Color(0, 1, 0, 1)):
					material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
					material.albedo_color = Color(0, 1, 0, 0)
				elif material.albedo_color.is_equal_approx(Color(0, 0, 1, 1)):
					material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
					material.albedo_color = Color(0, 0, 1, 0.5)

	for child in node.get_children():
		override_materials(child)


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
					var arrays := mesh.get_surface_arrays(surface_idx)
					var vertices: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
					var indices: PackedInt32Array = arrays[Mesh.ARRAY_INDEX]
					physics_mesh.add_mesh(
						node.position,
						node.rotation,
						node.scale,
						vertices,
						indices,
						material.albedo_color
					)

		var scene_node := document.generate_scene(state)
		override_materials(scene_node)
		scene_node.scale = Vector3(10.0, 10.0, 10.0)
		scene_node.set_meta("mesh", physics_mesh)
		loaded_track = scene_node
		loaded_mesh = physics_mesh

		scene_node.ready.connect(track_ready)

		add_child(scene_node)
	else:
		printerr("Couldn't load glTF scene (error code: %s)." % error_string(error))


func track_ready() -> void:
	level_loaded.rpc_id(1)
	$HUD.visible = true


func save_state() -> void:
	saved_state = loaded_player.save_state()


func load_state() -> void:
	if saved_state.size() > 0:
		loaded_player.load_state(saved_state)


func display_car_stats(speed: float, rpm: float, gear: int) -> void:
	$HUD/CarStats/Gear.text = "%d gear" % gear
	$HUD/CarStats/Speed.text = "%.f" % absf(speed * 3.6)
	$HUD/CarStats/Rpm.text = "%.f RPM" % rpm


func display_countdown(step: int) -> void:
	if step < 0:
		var seconds := step / 60.0
		seconds = absf(floorf(seconds))
		$HUD/Countdown.visible = true
		$HUD/Countdown.text = String.num(seconds)
	elif step <= 60:
		$HUD/Countdown.text = "Start"
	else:
		$HUD/Countdown.visible = false


func human_time(step: int, full: bool = false) -> String:
	var is_negative := step < 0
	step = absi(step)
	var seconds := step as float / 60.0
	var minutes := (seconds / 60.0) as int
	var remaining_seconds := fmod(seconds, 60.0)
	var prefix := "-" if is_negative else " "
	var time := "%s%02d:%06.3f" % [prefix, minutes, remaining_seconds]
	if !full:
		time[time.length() - 1] = " "

	return time


func checkpoint_text(collected: int, available: int) -> String:
	if available > 0:
		return "%s / %s" % [collected, available]
	else:
		return ""


func _on_spectated_player_simulated(step: CarPhysicsStep) -> void:
	if step.simulated:
		display_car_stats(step.speed, step.rpm, step.gear)
		display_countdown(step.step)
		$HUD/TrackStats/Time.text = human_time(step.step, step.just_finished)
		$HUD/TrackStats/Checkpoints.text = checkpoint_text(
			step.collected_checkpoints, step.available_checkpoints
		)


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


func _on_finish_menu_restart() -> void:
	if loaded_player:
		loaded_player.restart()


func _on_finish_menu_save_replay() -> void:
	if loaded_player:
		var replay := loaded_player.get_replay()
		var result := OK
		var replay_name := Time.get_datetime_string_from_system(false, true)

		result = DirAccess.make_dir_recursive_absolute("user://replays")
		if result == OK:
			result = ResourceSaver.save(
				replay, "user://replays/%s.res" % replay_name, ResourceSaver.FLAG_COMPRESS
			)

		if result == OK:
			$menu.set_replay_label("Replay saved as '%s'" % replay_name)
		else:
			printerr("Could not save replay (error: %s)" % error_string(result))
			$menu.set_replay_label("Error %s" % error_string(result))


func _on_pause_menu_load_replay() -> void:
	change_menu(MenuState.LOAD_REPLAY)


func _on_replay_menu_replay_toggled(replay_uri: String, toggled: bool) -> void:
	# Make replay player id negative as normal players are guaranteed to be positive
	# so they do not collide
	var player_id := -replay_uri.hash()
	var player_name := "Replay '%s'" % replay_uri.get_file().get_basename()
	if toggled:
		var replay := ResourceLoader.load(replay_uri) as Replay
		if !replay:
			printerr("Failed to load replay %s" % replay_uri)
			return

		spawn_player(player_id, player_name, PackedByteArray())
		var replay_player := loaded_track.get_node_or_null(str(player_id)) as Player
		if replay_player:
			replay_player.play_replay(replay)
	else:
		despawn_player(player_id)
