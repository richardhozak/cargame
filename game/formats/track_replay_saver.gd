@tool
class_name TrackReplaySaver extends ResourceFormatSaver

const VERSION = 1


func _get_recognized_extensions(_resource: Resource) -> PackedStringArray:
	return PackedStringArray(["replay"])


func _recognize(resource: Resource) -> bool:
	resource = resource as TrackReplay

	if resource:
		return true

	return false


func _save(resource: Resource, path: String = "", _flags: int = 0) -> Error:
	resource = resource as TrackReplay

	var file := FileAccess.open_compressed(path, FileAccess.WRITE, FileAccess.COMPRESSION_ZSTD)
	if !file:
		return FileAccess.get_open_error()

	resource.save_to(file)
	file.close()

	return OK
