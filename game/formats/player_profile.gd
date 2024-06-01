class_name PlayerProfile extends Resource

var player_name: String
var player_id: String


func save_to(file: FileAccess):
	file.store_8(1)
	file.store_pascal_string(player_name)
	file.store_pascal_string(player_id)


static func load_from(file: FileAccess):
	var version := file.get_8()
	if version == 1:
		var player := PlayerProfile.new()
		player.player_name = file.get_pascal_string()
		player.player_id = file.get_pascal_string()
		return player
	return ERR_FILE_UNRECOGNIZED
