extends Node3D

const MAX_CLIENTS = 4
const PORT = 3535
const IP_ADDRESS = "127.0.0.1"

var ip_address = IP_ADDRESS
var port = PORT

@onready var server_browser: ServerBrowser = $ServerBrowser

var validating := false
var create_server := false
var selected_track_uri := ""
var loaded_track: Node3D
var loaded_mesh: CarPhysicsTrackMesh
var player_name: String
var saved_state: PackedByteArray
var spectate_group := ButtonGroup.new()
var fastest_validation_replay := Replay.new()
var loaded_track_bytes: PackedByteArray
var track_res: Track
var personal_best: Replay

const PlayerScene = preload("res://player.tscn")
const PlayerSpectateItem = preload("res://player_spectate_item.tscn")
var loaded_player: Player

var current_menu_state: MenuState = MenuState.MAIN_MENU

enum MenuState {
	NONE,
	MAIN_MENU,
	TRACK_SELECT_HOST,
	TRACK_SELECT_SINGLE_PLAYER,
	PAUSED,
	FINISHED,
	FINISHED_WITH_VALIDATION,
	LOAD_REPLAY,
	LOAD_TRACK_MODEL,
	CREATE_TRACK,
	JOIN,
}


class PeerState:
	var name: String = ""
	var level_loaded: bool = false
	var player: Player = null

	func _init(peer_name: String) -> void:
		name = peer_name


var peers := Dictionary()


func _ready() -> void:
	spectate_group.allow_unpress = false
	spectate_group.pressed.connect(_on_spectate_pressed)
	player_name = RandomName.get_random_name()
	$PlayerName/PlayerNameContainer/PlayerName.text = player_name
	$DebugMenu/Menu/SaveStateButton.pressed.connect(save_state)
	$DebugMenu/Menu/LoadStateButton.pressed.connect(load_state)
	change_menu(MenuState.MAIN_MENU)


func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("change_camera"):
		if $FlyCamera.priority == 2:
			$FlyCamera.priority = 0
			var spectated_button := spectate_group.get_pressed_button()
			if spectated_button && spectated_button.has_meta("peer_id"):
				spectate(spectated_button.get_meta("peer_id"))
		else:
			$FlyCamera.priority = 2


func change_menu(menu_state: MenuState) -> void:
	var current_menu := get_node_or_null("menu")
	if current_menu:
		# change the name so it does not collide with newly spawned menu
		current_menu.name = "old_menu"
		current_menu.queue_free()

	match menu_state:
		MenuState.MAIN_MENU:
			match current_menu_state:
				MenuState.PAUSED, MenuState.FINISHED, MenuState.FINISHED_WITH_VALIDATION:
					prints("Disconnect from game")
					disconnect_from_game()
			create_server = false
			fastest_validation_replay = Replay.new()

			var menu := preload("res://menus/main_menu.tscn").instantiate()
			menu.name = "menu"
			menu.single_player.connect(_on_main_menu_single_player)
			menu.create_track.connect(_on_main_menu_create_track)
			menu.host.connect(_on_main_menu_host)
			menu.join.connect(_on_main_menu_join)
			menu.quit.connect(_on_main_menu_quit)
			add_child(menu)
		MenuState.TRACK_SELECT_HOST:
			create_server = true
			var menu := preload("res://menus/select_track_menu.tscn").instantiate()
			menu.name = "menu"
			menu.selected.connect(_on_select_track_menu_selected)
			add_child(menu)
		MenuState.TRACK_SELECT_SINGLE_PLAYER:
			create_server = false
			var menu := preload("res://menus/select_track_menu.tscn").instantiate()
			menu.name = "menu"
			menu.selected.connect(_on_select_track_menu_selected)
			add_child(menu)
		MenuState.PAUSED:
			if current_menu_state == MenuState.NONE:
				pause_children(true)

			var menu := preload("res://menus/pause_menu.tscn").instantiate()
			menu.name = "menu"
			menu.load_replay_visible = !validating
			menu.accept_validation_visible = validating
			menu.accept_validation_enabled = fastest_validation_replay.get_count() != 0
			menu.resume.connect(_on_pause_menu_resume)
			menu.load_replay.connect(_on_pause_menu_load_replay)
			menu.accept.connect(_on_finish_with_validation_menu_accept)
			menu.main_menu.connect(_on_pause_menu_main_menu)
			menu.quit.connect(_on_pause_menu_quit)
			add_child(menu)
		MenuState.FINISHED:
			var menu := preload("res://menus/finish_menu.tscn").instantiate()
			menu.name = "menu"
			menu.restart.connect(_on_finish_menu_restart)
			menu.save_replay.connect(_on_finish_menu_save_replay)
			menu.main_menu.connect(_on_pause_menu_main_menu)
			menu.quit.connect(_on_pause_menu_quit)
			add_child(menu)
		MenuState.FINISHED_WITH_VALIDATION:
			var menu := preload("res://menus/finish_with_validation_menu.tscn").instantiate()
			menu.name = "menu"
			menu.restart.connect(_on_finish_menu_restart)
			menu.accept.connect(_on_finish_with_validation_menu_accept)
			menu.main_menu.connect(_on_pause_menu_main_menu)
			menu.quit.connect(_on_pause_menu_quit)
			add_child(menu)
		MenuState.NONE:
			if current_menu_state == MenuState.PAUSED:
				pause_children(false)
		MenuState.LOAD_REPLAY:
			var menu := preload("res://menus/load_replay_menu.tscn").instantiate()
			menu.name = "menu"

			var replay_uris = get_tree().get_nodes_in_group("replays").map(
				func(node): return node.get_meta("replay_uri", "")
			)
			menu.selected_replay_uris.assign(replay_uris)
			menu.track_id = track_res.track_id
			menu.replay_toggled.connect(_on_replay_menu_replay_toggled)
			add_child(menu)
		MenuState.LOAD_TRACK_MODEL:
			var menu := preload("res://menus/load_track_model_menu.tscn").instantiate()
			menu.name = "menu"
			menu.file_selected.connect(_on_load_track_model_menu_file_selected)
			menu.canceled.connect(_on_load_track_model_menu_canceled)
			add_child(menu)
		MenuState.CREATE_TRACK:
			var menu := preload("res://menus/create_track_menu.tscn").instantiate()
			menu.name = "menu"
			menu.track_bytes = loaded_track_bytes
			menu.author_time = fastest_validation_replay.get_count() - 300
			menu.author_name = player_name
			menu.done.connect(_on_create_track_menu_done)
			add_child(menu)
		MenuState.JOIN:
			var menu := preload("res://menus/join_menu.tscn").instantiate()
			menu.name = "menu"
			menu.join_server.connect(_on_join_menu_join)
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

	player.spectate()


@rpc("any_peer", "call_local", "reliable")
func hello(peer_name: String) -> void:
	var remote_id := multiplayer.get_remote_sender_id()
	prints("got hello from", remote_id, peer_name)
	prints("sending load level")
	peers[remote_id] = PeerState.new(peer_name)
	load_level.rpc_id(remote_id, selected_track_uri)


@rpc("authority", "call_local", "reliable")
func load_level(track_uri: String) -> void:
	prints("loading level", track_uri, multiplayer.is_server())
	load_track(track_uri)


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
	var is_replay := id < 0
	var is_local := multiplayer.get_unique_id() == id
	prints("spawn player", id, peer_name, "is server", multiplayer.is_server())
	var player := PlayerScene.instantiate()
	player.name = str(id)
	player.player_name = peer_name
	player.initial_state = initial_state

	if is_replay:
		player.add_to_group("replays")

	var spectate_item := PlayerSpectateItem.instantiate()
	spectate_item.button_group = spectate_group
	spectate_item.player_name = peer_name
	$HUD/ScrollContainer/SpectateContainer.add_child(spectate_item)

	var spectate_button: BaseButton = spectate_group.get_buttons().back()
	spectate_button.set_meta("peer_id", id)

	loaded_track.add_child(player)

	if is_replay:
		return

	if is_local:
		player.driver = LocalPlayerInput.new()
		loaded_player = player
		spectate_button.button_pressed = true
		if personal_best && is_playing_single_player():
			spawn_player(-id, "Personal Best", PackedByteArray())
			var personal_best_player := loaded_track.get_node_or_null(str(-id)) as Player
			if personal_best_player:
				personal_best_player.driver = ReplayPlayerInput.new(personal_best)
	else:
		player.driver = RemotePlayerInput.new()

	# bind inputs simulated inputs, server distributes them to clients, clients just need to send them to server
	if multiplayer.is_server():
		player.simulated.connect(func(step): on_server_input_simulated(id, step.input))

	if is_local:
		player.simulated.connect(func(step): on_local_step_simulated(step))

	if multiplayer.is_server():
		peers[id].player = player


@rpc("any_peer", "call_remote", "reliable")
func input_simulated(input: CarPhysicsInput) -> void:
	var peer_id := multiplayer.get_remote_sender_id()
	var node := loaded_track.get_node_or_null(str(peer_id)) as Player
	if node:
		var driver := node.driver as RemotePlayerInput
		driver.queue_input(input)
	else:
		printerr("no player with id ", peer_id)


@rpc("authority", "call_local", "reliable")
func simulate_input(peer_id: int, input: CarPhysicsInput) -> void:
	var node := loaded_track.get_node_or_null(str(peer_id)) as Player
	if node:
		var driver := node.driver as RemotePlayerInput
		driver.queue_input(input)
	else:
		printerr("no player with id ", peer_id)


func on_local_step_simulated(step: CarPhysicsStep) -> void:
	if !multiplayer.is_server():
		# send input to server
		input_simulated.rpc_id(1, step.input)

	if step.just_finished:
		var driver := loaded_player.driver as LocalPlayerInput
		if validating:
			var current_replay := driver.get_replay()
			if (
				(current_replay.get_count() <= fastest_validation_replay.get_count())
				|| fastest_validation_replay.get_count() == 0
			):
				fastest_validation_replay = current_replay

			var fastest_time := Replays.human_time(
				fastest_validation_replay.get_count() - 300, true
			)
			$HUD/ValidatingLabel.text = "Validated with" + fastest_time
			change_menu(MenuState.FINISHED_WITH_VALIDATION)
			$menu.set_fastest_time(fastest_time)
			$menu.set_time(Replays.human_time(step.step, true))
		else:
			var achievement := ""
			var current_replay := driver.get_replay()
			if (
				(personal_best and current_replay.get_count() <= personal_best.get_count())
				or !personal_best
			):
				achievement = "New personal best!"

				var old_medal := ""
				if personal_best:
					old_medal = get_time_medal(personal_best.get_count() - 300)

				var new_medal := get_time_medal(current_replay.get_count() - 300)

				if old_medal != new_medal:
					achievement = "You've earned new medal: %s!" % new_medal

				personal_best = current_replay

				var result := Replays.save_personal_best(
					track_res.track_id, player_name, personal_best
				)
				if result.result != OK:
					printerr("Failed to save PB %s", error_string(result.result))

				var pb_id := str(-multiplayer.get_unique_id())
				var personal_best_player: Player = loaded_track.get_node_or_null(pb_id)
				if !personal_best_player && is_playing_single_player():
					spawn_player(-multiplayer.get_unique_id(), "Personal Best", PackedByteArray())
					personal_best_player = loaded_track.get_node(pb_id)

				personal_best_player.driver = ReplayPlayerInput.new(personal_best)

			change_menu(MenuState.FINISHED)
			$menu.set_time(Replays.human_time(step.step, step.just_finished))
			$menu.set_personal_best(Replays.human_time(personal_best.get_count() - 300, true))
			$menu.set_achievement(achievement)

	if step.input.restart:
		match current_menu_state:
			MenuState.FINISHED, MenuState.FINISHED_WITH_VALIDATION:
				change_menu(MenuState.NONE)

		get_tree().call_group("replays", "restart")


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

	var despawned_player := loaded_track.get_node_or_null(str(id)) as Player
	if despawned_player:
		if despawned_player.is_spectated():
			spectate(str(loaded_player.name).to_int())

		despawned_player.queue_free()


func peer_connected(_id: int) -> void:
	# TODO: timeout peer if they do not send 'hello'
	return


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
	if loaded_track:
		loaded_track.queue_free()
		loaded_track = null
	multiplayer.multiplayer_peer = null
	server_browser.unregister()


func host_game() -> void:
	# Create server.
	assert(multiplayer is SceneMultiplayer)
	multiplayer.server_relay = false
	multiplayer.allow_object_decoding = true

	if create_server:
		var peer := ENetMultiplayerPeer.new()
		peer.create_server(0, MAX_CLIENTS)
		multiplayer.multiplayer_peer = peer
		multiplayer.peer_connected.connect(peer_connected)
		multiplayer.peer_disconnected.connect(peer_disconnected)
		server_browser.register(peer.host.get_local_port(), {"host": player_name})
	else:
		multiplayer.multiplayer_peer = OfflineMultiplayerPeer.new()

	hello.rpc_id(1, player_name)


func is_playing_single_player() -> bool:
	return multiplayer.multiplayer_peer is OfflineMultiplayerPeer


func join_game() -> void:
	# Create client.
	multiplayer.allow_object_decoding = true
	var peer := ENetMultiplayerPeer.new()
	peer.create_client(ip_address, port)

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


func load_track_from_bytes(buffer: PackedByteArray) -> Error:
	var document := GLTFDocument.new()
	var state := GLTFState.new()
	var error := document.append_from_buffer(buffer, "", state)
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
	return error


func load_track(track_uri: String) -> void:
	if loaded_track != null:
		remove_child(loaded_track)
		loaded_track = null
		loaded_mesh = null

	if personal_best != null:
		personal_best = null

	if validating:
		if track_uri.get_extension() == "glb" || track_uri.get_extension() == "gltf":
			var buffer := FileAccess.get_file_as_bytes(track_uri)
			var error := FileAccess.get_open_error()
			if error == OK:
				print("Load track")
				error = load_track_from_bytes(buffer)
				if error == OK:
					loaded_track_bytes = buffer
				else:
					printerr("Couldn't load glTF scene (error code: %s)." % error_string(error))
			else:
				printerr("Error while opening track %s" % error_string(error))
		else:
			printerr("Invalid file")
	else:
		var track := ResourceLoader.load(track_uri) as Track
		if track:
			personal_best = Replays.get_personal_best(track.track_id)
			var error := load_track_from_bytes(track.track_bytes)
			if error == OK:
				track_res = track
			else:
				personal_best = null
				printerr("Couldn't load track scene (error code: %s)." % error_string(error))
		else:
			printerr("Failed to load track")


func track_ready() -> void:
	level_loaded.rpc_id(1)
	$HUD/ValidatingLabel.text = "Validating" if validating else ""
	$HUD.visible = true


func save_state() -> void:
	saved_state = loaded_player.save_state()


func load_state() -> void:
	if saved_state.size() > 0:
		loaded_player.load_state(saved_state)


func _on_main_menu_host() -> void:
	change_menu(MenuState.TRACK_SELECT_HOST)


func _on_main_menu_join() -> void:
	change_menu(MenuState.JOIN)


func _on_main_menu_quit() -> void:
	get_tree().quit()


func _on_select_track_menu_selected(track_uri: String) -> void:
	change_menu(MenuState.NONE)
	validating = false
	selected_track_uri = track_uri
	host_game()


func _on_join_menu_join(server: ServerInfo) -> void:
	change_menu(MenuState.NONE)
	ip_address = server.addresses[0].ip
	port = server.addresses[0].port
	join_game()


func _on_main_menu_single_player() -> void:
	change_menu(MenuState.TRACK_SELECT_SINGLE_PLAYER)


func _on_main_menu_create_track() -> void:
	change_menu(MenuState.LOAD_TRACK_MODEL)


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
	if loaded_player && track_res:
		var driver := loaded_player.driver as LocalPlayerInput
		var replay := driver.get_replay()
		var result := Replays.save_replay(track_res.track_id, player_name, replay)
		$menu.set_replay_label(result.message)


func _on_finish_with_validation_menu_accept() -> void:
	change_menu(MenuState.CREATE_TRACK)


func _on_load_track_model_menu_file_selected(file: String):
	change_menu(MenuState.NONE)
	validating = true
	selected_track_uri = file
	host_game()


func _on_load_track_model_menu_canceled():
	change_menu(MenuState.MAIN_MENU)


func _on_pause_menu_load_replay() -> void:
	change_menu(MenuState.LOAD_REPLAY)


func _on_create_track_menu_done() -> void:
	change_menu(MenuState.MAIN_MENU)


func get_time_medal(step: int) -> String:
	if step <= track_res.author_time:
		return "Author"
	elif step <= track_res.gold_time:
		return "Gold"
	elif step <= track_res.silver_time:
		return "Silver"
	elif step <= track_res.bronze_time:
		return "Bronze"
	else:
		return ""


func _on_replay_menu_replay_toggled(replay_uri: String, toggled: bool) -> void:
	# Make replay player id negative as normal players are guaranteed to be positive
	# so they do not collide
	var player_id := -replay_uri.hash()
	var replay: TrackReplay = Replays.loaded_replays[replay_uri]
	var replay_player_name := "Replay '%s'" % replay.player_name
	if toggled:
		spawn_player(player_id, replay_player_name, PackedByteArray())
		var replay_player := loaded_track.get_node_or_null(str(player_id)) as Player
		if replay_player:
			replay_player.set_meta("replay_uri", replay_uri)
			replay_player.driver = ReplayPlayerInput.new(replay.replay)
	else:
		despawn_player(player_id)

	_apply_replay_colors()


func _apply_replay_colors():
	# this function assigns replay car colors according to their time,
	# the faster they are the lighter shade of blue they are

	var replays := get_tree().get_nodes_in_group("replays")
	replays.sort_custom(sort_slower)

	if replays.size() >= 2:
		var slowest := replays.front().driver.get_replay().get_count() as int
		var fastest := replays.back().driver.get_replay().get_count() as int
		var diff := slowest - fastest
		var one_percent := diff as float / 100.0

		for player in replays:
			var count := player.driver.get_replay().get_count() as int
			var percent_slow := (count - fastest) / one_percent
			var percent_fast := 100.0 - percent_slow
			var ratio := percent_fast / 100.0
			var lighter_amount := ratio * 0.6
			player.color = Color.BLUE.lightened(lighter_amount)


func sort_slower(a: Player, b: Player) -> bool:
	return a.driver.get_replay().get_count() > b.driver.get_replay().get_count()
