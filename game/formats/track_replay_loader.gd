@tool
class_name TrackReplayLoader extends ResourceFormatLoader


func _get_recognized_extensions() -> PackedStringArray:
	return PackedStringArray(["replay"])


func _get_resource_type(path: String) -> String:
	var ext = path.get_extension().to_lower()

	if ext == "replay":
		return "Resource"

	return ""


func _handles_type(typename: StringName) -> bool:
	return ClassDB.is_parent_class(typename, "Resource")


func _load(path: String, original_path: String, use_sub_threads: bool, cache_mode: int):
	var file := FileAccess.open_compressed(path, FileAccess.READ, FileAccess.COMPRESSION_ZSTD)

	if !file:
		printerr("Error opening replay file: ", FileAccess.get_open_error())
		return FileAccess.get_open_error()

	return TrackReplay.load_from(file)
