class_name Replay extends Resource

const INPUT_SIZE: int = 7

@export var count: int
@export var data: PackedByteArray


func get_count() -> int:
	return count


func add_input(input: CarPhysicsInput) -> void:
	data.push_back(input.up)
	data.push_back(input.down)
	data.push_back(input.left)
	data.push_back(input.right)
	data.push_back(input.brake)
	data.push_back(input.respawn)
	data.push_back(input.restart)
	count += 1


func get_input(index: int) -> CarPhysicsInput:
	var input := CarPhysicsInput.new()
	var begin := index * INPUT_SIZE
	assert(begin < data.size())
	input.up = data.decode_u8(begin + 0)
	input.down = data.decode_u8(begin + 1)
	input.left = data.decode_u8(begin + 2)
	input.right = data.decode_u8(begin + 3)
	input.brake = data.decode_u8(begin + 4)
	input.respawn = data.decode_u8(begin + 5)
	input.restart = data.decode_u8(begin + 6)
	return input
