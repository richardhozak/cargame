class_name UUIDv4 extends RefCounted

static var _crypto := Crypto.new()


static func generate() -> String:
	var bytes := _crypto.generate_random_bytes(16)
	assert(bytes.size() == 16)
	bytes[6] = (bytes[6] & 0x0f) | 0x40
	bytes[8] = (bytes[8] & 0x3f) | 0x80
	var id := "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x" % Array(bytes)
	print("UUIDv4 ", id)
	return id
