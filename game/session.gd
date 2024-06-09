extends Node

const API_KEY := "dev_109b0bf1efd34b65bca7f1040dc199eb"
const PROFILE_DIR := "user://profiles"

var player_profile: PlayerProfile

@onready var _profile := _get_profile()
var _session_token: String
var _http_request := HTTPRequest.new()


func _ready():
	_http_request.timeout = 10.0
	add_child(_http_request)

	if _profile_exists(_profile):
		player_profile = _load_player_profile(_profile)
	else:
		print("Creating new profile")
		player_profile = _create_new_player_profile()
		var result := _save_player_profile(_profile, player_profile)
		if result != OK:
			printerr("Failed to save player profile %s" % error_string(result))
			return

	if player_profile:
		print("Player profile '%s' loaded %s" % [_profile, player_profile.player_name])
	else:
		printerr("Failed to load player profile %s" % error_string(FileAccess.get_open_error()))

	assert(player_profile, "Player profile not available")
	_login()


func _parse_json_response_body(bytes: PackedByteArray):
	var json_body := JSON.new()
	json_body.parse(bytes.get_string_from_utf8())
	var body = json_body.get_data()
	if !body:
		printerr("Invalid json response body")
		return null

	return body


func _login() -> void:
	print("Creating guest session ", player_profile.player_id)

	var request := {"game_key": API_KEY, "game_version": "0.1.0"}

	if player_profile.player_id:
		print("Logging in with ", player_profile.player_id)
		request["player_identifier"] = player_profile.player_id

	_http_request.request(
		"https://api.lootlocker.io/game/v2/session/guest",
		["content-type: application/json"],
		HTTPClient.METHOD_POST,
		JSON.stringify(request)
	)

	var response = await _http_request.request_completed

	var result := response[0] as HTTPRequest.Result
	if result != HTTPRequest.Result.RESULT_SUCCESS:
		printerr("Failed to get online user profile")
		return

	var body := _parse_json_response_body(response[3] as PackedByteArray) as Dictionary
	if !body:
		printerr("Invalid response from login")
		return

	var player_name := body.get("player_name", "") as String
	player_profile.player_id = body.get("player_identifier", "") as String
	_session_token = body.get("session_token", "") as String

	var save_result := _save_player_profile(_profile, player_profile)
	if save_result != OK:
		print("Failed to save profile %s", error_string(save_result))
		return

	print("Player logged in with ", player_profile.player_id)

	if not player_name or player_name != player_profile.player_name:
		prints("Update player name from '%s' to '%s'" % [player_name, player_profile.player_name])
		await _update_player_name(player_profile.player_name)


func _update_player_name(player_name: String):
	var http_result := _http_request.request(
		"https://api.lootlocker.io/game/player/name",
		["content-type: application/json", "x-session-token: %s" % _session_token],
		HTTPClient.METHOD_PATCH,
		JSON.stringify({"name": player_name})
	)

	if http_result != HTTPRequest.Result.RESULT_SUCCESS:
		printerr("Failed to update player's name: ", http_result)
		return

	var response = await _http_request.request_completed

	var result := response[0] as HTTPRequest.Result
	if result != HTTPRequest.Result.RESULT_SUCCESS:
		printerr("Failed to update player's name: ", result)
		return

	var body = _parse_json_response_body(response[3] as PackedByteArray)
	if !body:
		printerr("Invalid response from update name")
		return

	if body.get("name", "") != player_name:
		printerr("Name update failed ", body)
	else:
		print("Name updated ", body)


func _parse_cmdline_args() -> Dictionary:
	var cmdline := OS.get_cmdline_user_args()
	print("Args ", cmdline)
	var arguments = {}
	for argument in cmdline:
		if argument.find("=") > -1:
			var key_value = argument.split("=")
			arguments[key_value[0].lstrip("--")] = key_value[1]
		else:
			# Options without an argument will be present in the dictionary,
			# with the value set to an empty string.
			arguments[argument.lstrip("--")] = ""
	return arguments


func _get_profile() -> String:
	_ensure_profile_dir_exists()

	var args := _parse_cmdline_args()
	var profile := args.get("profile", "") as String

	if profile:
		return profile

	# default profile is called "player"
	return "player"


func _profile_exists(profile: String) -> bool:
	return ResourceLoader.exists("%s/%s.profile" % [PROFILE_DIR, profile])


func _load_player_profile(profile: String) -> PlayerProfile:
	print("Loading profile '%s'" % profile)
	return ResourceLoader.load("%s/%s.profile" % [PROFILE_DIR, profile])


func _save_player_profile(profile: String, player: PlayerProfile) -> Error:
	print("Saving profile '%s'" % profile)
	return ResourceSaver.save(player, "%s/%s.profile" % [PROFILE_DIR, profile])


func _create_new_player_profile():
	var player := PlayerProfile.new()
	player.player_name = RandomName.get_random_name()
	player.player_id = UUIDv4.generate()
	return player


func _ensure_profile_dir_exists():
	var result := DirAccess.make_dir_recursive_absolute(PROFILE_DIR)
	if result != OK:
		push_error("Failed to create profile dir: %s" % error_string(result))
		assert(false, "Failed to create profile dir: %s" % error_string(result))


# curl -X POST "https://api.lootlocker.io/game/leaderboards/1/submit" \
#   -H "x-session-token: your_token_here" \
#   -H "Content-Type: application/json" \
#   -d "{\"score\": 1000, \"metadata\": \"some metadata\"}"
func submit_replay(track_id: String, replay: Replay) -> void:
	if not _session_token:
		printerr("Cannot submit replay, session token not set")
		return

	assert(replay.data.size() == replay.count * Replay.INPUT_SIZE)
	var data := replay.data.compress(FileAccess.CompressionMode.COMPRESSION_ZSTD)

	print("Submitting replay")

	var error := _http_request.request(
		"https://api.lootlocker.io/game/leaderboards/%s/submit" % track_id,
		["content-type: application/json", "x-session-token: %s" % _session_token],
		HTTPClient.METHOD_POST,
		JSON.stringify({"score": replay.count, "metadata": Marshalls.raw_to_base64(data)})
	)
	if error != OK:
		printerr("Failed to submit replay, error: ", error_string(error))
		return

	var response = await _http_request.request_completed

	var result := response[0] as HTTPRequest.Result
	if result != HTTPRequest.Result.RESULT_SUCCESS:
		printerr("Failed to submit replay, status: ", result)
		return

	var body = _parse_json_response_body(response[3] as PackedByteArray) as Dictionary
	if !body:
		printerr("Failed to submit replay, failed to parse reponse")
		return

	if body.has("message"):
		printerr("Failed to submit replay ", body.get("message"))
	else:
		print("Replay submitted")


# curl -X GET "https://api.lootlocker.io/game/leaderboards/1/list?count=10"
func get_leaderboard(track_id: String):
	var error := _http_request.request(
		"https://api.lootlocker.io/game/leaderboards/%s/list/?count=10" % track_id,
		["x-session-token: %s" % _session_token]
	)

	if error != OK:
		printerr("Failed to get leaderboard for track '%s': %s" % [track_id, error_string(error)])
		return null

	var response = await _http_request.request_completed

	var result := response[0] as HTTPRequest.Result
	if result != HTTPRequest.Result.RESULT_SUCCESS:
		printerr("Failed to get leaderboard for track, status: ", result)
		return null

	var body = _parse_json_response_body(response[3] as PackedByteArray) as Dictionary
	if !body:
		printerr("Failed to submit replay, failed to parse reponse")
		return null

	if body.has("items"):
		return body.get("items")
	else:
		printerr("Failed to get leaderboard ", body)
		return null
