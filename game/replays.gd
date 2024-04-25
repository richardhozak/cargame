extends Node


class SaveResult:
	var result: Error
	var message: String

	func _init(p_result: Error, p_message := ""):
		if p_message.is_empty() && p_result != OK:
			message = "Error %s" % error_string(p_result)
		else:
			message = p_message

		result = p_result


var loaded_replays: Dictionary


func _ready():
	var replay_dir := "user://replays"
	var replay_files := DirAccess.get_files_at(replay_dir)
	for replay_file in replay_files:
		var replay_uri = "%s/%s" % [replay_dir, replay_file]
		var replay := ResourceLoader.load(replay_uri) as TrackReplay
		if !replay:
			printerr("Failed to load replay %s" % replay_file)
			continue

		loaded_replays[replay_uri] = replay


func save_replay(track_name: String, player_name: String, replay: Replay) -> SaveResult:
	var track_replay := TrackReplay.new()
	track_replay.track_name = track_name
	track_replay.player_name = player_name
	track_replay.replay = replay

	var result := OK
	var replay_name := Time.get_datetime_string_from_system(false, true)
	var replay_dir := "user://replays"
	result = DirAccess.make_dir_recursive_absolute(replay_dir)
	if result != OK:
		return SaveResult.new(result)

	var replay_uri = "%s/%s.res" % [replay_dir, replay_name]
	result = ResourceSaver.save(track_replay, replay_uri, ResourceSaver.FLAG_COMPRESS)

	if result != OK:
		printerr("Could not save replay (error: %s)" % error_string(result))
		return SaveResult.new(result)

	loaded_replays[replay_uri] = track_replay
	return SaveResult.new(OK, "Replay saved as '%s'" % replay_name)
