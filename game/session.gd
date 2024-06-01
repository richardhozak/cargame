extends Node

const PROFILE_DIR := "user://profiles"

var player_profile: PlayerProfile


func _ready():
	_ensure_profile_dir_exists()

	var profile := _get_profile()

	if _profile_exists(profile):
		player_profile = _load_player_profile(profile)
	else:
		print("Creating new profile")
		player_profile = _create_new_player_profile()
		var result := _save_player_profile(profile, player_profile)
		if result != OK:
			printerr("Failed to save player profile %s" % error_string(result))
			return

	if player_profile:
		print("Player profile '%s' loaded %s" % [profile, player_profile.player_name])
	else:
		printerr("Failed to load player profile %s" % error_string(FileAccess.get_open_error()))

	assert(player_profile, "Player profile not available")


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
	player.player_id = ""
	return player


func _ensure_profile_dir_exists():
	var result := DirAccess.make_dir_recursive_absolute(PROFILE_DIR)
	if result != OK:
		push_error("Failed to create profile dir: %s" % error_string(result))
		assert(false, "Failed to create profile dir: %s" % error_string(result))
