@tool
class_name PlayerProfileLoader extends ResourceFormatLoader


func _get_recognized_extensions() -> PackedStringArray:
	return PackedStringArray(["profile"])


func _get_resource_type(path: String) -> String:
	var ext := path.get_extension().to_lower()

	if ext == "profile":
		return "Resource"

	return ""


func _handles_type(typename: StringName) -> bool:
	return ClassDB.is_parent_class(typename, "Resource")


func _load(path: String, _original_path: String, _use_sub_threads: bool, _cache_mode: int):
	var file := FileAccess.open_compressed(path, FileAccess.READ, FileAccess.COMPRESSION_ZSTD)

	if !file:
		return FileAccess.get_open_error()

	return PlayerProfile.load_from(file)
