class_name Replay extends Resource

const INPUT_SIZE: int = 7

@export var count: int
@export var data: PackedByteArray


func get_count() -> int:
	return count


func add_input(input: CarPhysicsInput) -> void:
	var input_bytes := PackedByteArray()
	assert(input_bytes.resize(INPUT_SIZE) == OK)
	input_bytes.encode_u8(0, input.up)
	input_bytes.encode_u8(1, input.down)
	input_bytes.encode_u8(2, input.left)
	input_bytes.encode_u8(3, input.right)
	input_bytes.encode_u8(4, input.brake)
	input_bytes.encode_u8(5, input.respawn)
	input_bytes.encode_u8(6, input.restart)
	data.append_array(input_bytes)
	count += 1


func get_input(index: int) -> CarPhysicsInput:
	var input := CarPhysicsInput.new()
	var begin := index * INPUT_SIZE
	var end := begin + INPUT_SIZE
	var input_bytes := data.slice(begin, end)
	assert(input_bytes.size() == INPUT_SIZE)
	input.up = input_bytes.decode_u8(0)
	input.down = input_bytes.decode_u8(1)
	input.left = input_bytes.decode_u8(2)
	input.right = input_bytes.decode_u8(3)
	input.brake = input_bytes.decode_u8(4)
	input.respawn = input_bytes.decode_u8(5)
	input.restart = input_bytes.decode_u8(6)
	return input
