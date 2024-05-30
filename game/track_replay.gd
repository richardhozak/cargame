class_name TrackReplay extends Resource

const VERSION := 1

@export var track_id: String
@export var player_name: String
@export var replay: Replay


func save_to(file: FileAccess) -> void:
	file.store_8(VERSION)
	file.store_pascal_string(track_id)
	file.store_pascal_string(player_name)
	file.store_32(replay.count)
	file.store_buffer(replay.data)


static func load_from(file: FileAccess):
	var version := file.get_8()
	if version == 1:
		var track_replay := TrackReplay.new()
		track_replay.track_id = file.get_pascal_string()
		track_replay.player_name = file.get_pascal_string()
		track_replay.replay = Replay.new()
		track_replay.replay.count = file.get_32()
		track_replay.replay.data = file.get_buffer(track_replay.replay.count * Replay.INPUT_SIZE)
		return track_replay
	else:
		return ERR_FILE_UNRECOGNIZED
