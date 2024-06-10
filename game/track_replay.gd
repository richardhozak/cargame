class_name TrackReplay extends Resource

const VERSION := 1

var track_id: String
var player_name: String
var player_id: String
var replay: Replay


func _init(
	p_track_id: String, p_player_name: String, p_player_id: String, p_replay: Replay
) -> void:
	track_id = p_track_id
	player_name = p_player_name
	player_id = p_player_id
	replay = p_replay.duplicate()


func save_to(file: FileAccess) -> void:
	file.store_8(VERSION)
	file.store_pascal_string(track_id)
	file.store_pascal_string(player_name)
	file.store_pascal_string(player_id)
	file.store_32(replay.count)
	file.store_buffer(replay.data)


static func load_from(file: FileAccess):
	var version := file.get_8()
	if version == 1:
		var p_track_id := file.get_pascal_string()
		var p_player_name := file.get_pascal_string()
		var p_player_id := file.get_pascal_string()
		var p_replay := Replay.new()
		p_replay.count = file.get_32()
		p_replay.data = file.get_buffer(p_replay.count * Replay.INPUT_SIZE)
		return TrackReplay.new(p_track_id, p_player_name, p_player_id, p_replay)
	else:
		return ERR_FILE_UNRECOGNIZED
