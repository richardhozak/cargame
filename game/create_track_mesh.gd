@tool
extends EditorScenePostImport

const TrackMesh = preload("res://track_mesh.gd")

# Called by the editor when a scene has this script set as the import script in the import tab.
func _post_import(scene):
	var track_mesh := TrackMesh.new()
	iterate(track_mesh, scene)
	print("created track mesh with vertices: ", len(track_mesh.vertices), ", indices: ", len(track_mesh.indices))
	# Modify the contents of the scene upon import.
	return scene # Return the modified root node when you're done.

func iterate(track_mesh: TrackMesh, node):
	if node is MeshInstance3D:
		var mesh: Mesh = node.mesh
		for index in mesh.get_surface_count():
			var arrays := mesh.surface_get_arrays(index)
			var vertices: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
			var indices: PackedInt32Array = arrays[Mesh.ARRAY_INDEX]
			track_mesh.add_mesh(vertices, indices)

	for child in node.get_children():
		iterate(track_mesh, child)
