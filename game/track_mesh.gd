extends Resource

@export var vertices: Array[PackedVector3Array]
@export var indices: Array[PackedInt32Array]


func add_mesh(vertex: PackedVector3Array, index: PackedInt32Array):
	vertices.append(vertex)
	indices.append(index)
