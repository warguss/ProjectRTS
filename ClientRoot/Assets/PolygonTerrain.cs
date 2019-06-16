using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class PolygonTerrain : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void Initialize(TerrainInfo terrainInfo)
    {
        var vertices3D = System.Array.ConvertAll<Vector2, Vector3>(terrainInfo.vertices2D, v => v);

        // Use the triangulator to get indices for creating triangles
        Triangulator triangulator = new Triangulator(terrainInfo.vertices2D);
        int[] indices = triangulator.Triangulate();

        // Generate a color for each vertex
        var colors = Enumerable.Range(0, vertices3D.Length)
            .Select(i => terrainInfo.TerrainColor)
            .ToArray();

        // Create the mesh
        var mesh = new Mesh
        {
            vertices = vertices3D,
            triangles = indices,
            colors = colors
        };

        mesh.RecalculateNormals();
        mesh.RecalculateBounds();

        // Set up game object with mesh;
        var meshRenderer = gameObject.AddComponent<MeshRenderer>();
        meshRenderer.material = new Material(Shader.Find("Sprites/Default"));

        var filter = gameObject.AddComponent<MeshFilter>();
        filter.mesh = mesh;

        var collider = gameObject.AddComponent<PolygonCollider2D>();
        collider.points = terrainInfo.vertices2D;
    }
}
