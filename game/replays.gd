extends Node

const REPLAY_DIR := "user://replays"
const PERSONAL_BESTS_DIR := "user://replays/pbs"

var loaded_replays: Dictionary
var personal_best_replays: Dictionary


func _ready() -> void:
	var replay_files := DirAccess.get_files_at(REPLAY_DIR)
	for replay_file in replay_files:
		var replay_uri := "%s/%s" % [REPLAY_DIR, replay_file]
		var replay := ResourceLoader.load(replay_uri) as TrackReplay
		if !replay:
			printerr("Failed to load replay %s" % replay_file)
			continue

		loaded_replays[replay_uri] = replay

	var personal_bests := DirAccess.get_files_at(PERSONAL_BESTS_DIR)
	for replay_file in personal_bests:
		var replay_uri := "%s/%s" % [PERSONAL_BESTS_DIR, replay_file]
		var replay := ResourceLoader.load(replay_uri) as TrackReplay
		if !replay:
			printerr("Failed to load replay %s" % replay_file)
			continue

		if personal_best_replays.has(replay.track_id):
			var stored_replay := personal_best_replays[replay.track_id] as TrackReplay
			if replay.replay.get_count() <= stored_replay.replay.get_count():
				personal_best_replays[replay.track_id] = replay
		else:
			personal_best_replays[replay.track_id] = replay


func save_personal_best(track_id: String, player_name: String, replay: Replay) -> SaveResult:
	return _save_replay(track_id, player_name, replay, PERSONAL_BESTS_DIR)


func save_replay(track_id: String, player_name: String, replay: Replay) -> SaveResult:
	var result := _save_replay(track_id, player_name, replay, REPLAY_DIR)
	if result.result == OK:
		loaded_replays[result.replay_uri] = result.replay

	return result


func get_personal_best(track_id: String) -> Replay:
	var replay := personal_best_replays.get(track_id) as TrackReplay
	if replay:
		return replay.replay
	else:
		return null


func _save_replay(
	track_id: String, player_name: String, replay: Replay, directory: String
) -> SaveResult:
	var track_replay := TrackReplay.new()
	track_replay.track_id = track_id
	track_replay.player_name = player_name
	track_replay.replay = replay

	var result := OK
	var replay_name := Time.get_datetime_string_from_system(false, true)
	result = DirAccess.make_dir_recursive_absolute(directory)
	if result != OK:
		return SaveResult.new(result)

	var replay_uri := "%s/%s.res" % [directory, replay_name]
	result = ResourceSaver.save(track_replay, replay_uri, ResourceSaver.FLAG_COMPRESS)

	if result != OK:
		printerr("Could not save replay (error: %s)" % error_string(result))
		return SaveResult.new(result)

	return SaveResult.new(OK, replay_uri, track_replay, "Replay saved as '%s'" % replay_name)


static func human_time(step: int, full: bool = false, pad: bool = true) -> String:
	var is_negative := step < 0
	step = absi(step)
	var seconds := step as float / 60.0
	var minutes := (seconds / 60.0) as int
	var remaining_seconds := fmod(seconds, 60.0)
	var prefix := "-" if is_negative else " "
	var time := "%s%02d:%06.3f" % [prefix, minutes, remaining_seconds]
	if !full:
		time[time.length() - 1] = " "

	if !pad:
		time = time.strip_edges()

	return time


class SaveResult:
	var result: Error
	var message: String
	var replay_uri: String
	var replay: TrackReplay

	func _init(
		p_result: Error, p_replay_uri := "", p_replay: TrackReplay = null, p_message := ""
	) -> void:
		replay_uri = p_replay_uri
		replay = p_replay

		if p_message.is_empty() && p_result != OK:
			message = "Error %s" % error_string(p_result)
		else:
			message = p_message

		result = p_result
