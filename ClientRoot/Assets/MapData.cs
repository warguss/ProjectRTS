using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TerrainInfo
{
    public Vector2[] vertices2D { get; set; }
    public Color TerrainColor;
}

public class MapData : MonoBehaviour {
    int xMax = 100;
    int yMax = 100;
    List<TerrainInfo> terrainList = new List<TerrainInfo>();

    public GameObject PolygonTerrainPrefab;

	// Use this for initialization
	void Start ()
    {
        
    }

    public void LoadMap()
    {
        AddBox(new Vector2(0, 0), new Vector2(100, 1));
        AddBox(new Vector2(99, 1), new Vector2(100, 100));
        AddBox(new Vector2(0, 99), new Vector2(100, 100));
        AddBox(new Vector2(0, 0), new Vector2(1, 100));

        for(int i=0; i<20; i++)
        {
            for(int j=0; j<25; j++)
            {
                AddBox(new Vector2(i * 5 + 4, j * 4 + 4), new Vector2(i * 5 + 4 + 3, j * 4 + 5));
            }
        }
    }

    public void DrawMap()
    {
        var MapTiles = new GameObject("MapTiles");

        for (int i=0; i<terrainList.Count; i++)
        {
            var polygonTerrain = Instantiate(PolygonTerrainPrefab).GetComponent<PolygonTerrain>();
            polygonTerrain.Initialize(terrainList[i]);
        }
    }

    private void AddBox(Vector2 from, Vector2 to)
    {
        var terrain = new TerrainInfo();
        terrain.vertices2D = new Vector2[] {
            new Vector2(from.x,from.y),
            new Vector2(from.x, to.y),
            new Vector2(to.x, to.y),
            new Vector2(to.x ,from.y),
        };
        terrain.TerrainColor = new Color(255, 255, 255);
        terrainList.Add(terrain);
    }

    private void AddPolygon(Vector2[] vertices2D)
    {
        var terrain = new TerrainInfo();
        terrain.vertices2D = vertices2D;
        terrain.TerrainColor = new Color(255, 255, 255);
        terrainList.Add(terrain);
    }
}
