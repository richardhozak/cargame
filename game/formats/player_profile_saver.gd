@tool
class_name PlayerProfileSaver extends ResourceFormatSaver


func _get_recognized_extensions(_resource: Resource) -> PackedStringArray:
	return PackedStringArray(["profile"])


func _recognize(resource: Resource) -> bool:
	resource = resource as PlayerProfile

	if resource:
		return true

	return false


func _save(resource: Resource, path: String = "", _flags: int = 0) -> Error:
	resource = resource as PlayerProfile

	var file := FileAccess.open_compressed(path, FileAccess.WRITE, FileAccess.COMPRESSION_ZSTD)
	if !file:
		return FileAccess.get_open_error()

	resource.save_to(file)
	file.close()

	return OK
