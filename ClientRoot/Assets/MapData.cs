using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MapData : MonoBehaviour {
    class TileInfo
    {
        public Vector2 From { get; set; }
        public Vector2 To { get; set; }
    }


    int xMax = 100;
    int yMax = 100;
    List<TileInfo> tileList = new List<TileInfo>();

    public GameObject UnitTile;

	// Use this for initialization
	void Start ()
    {
        
    }

    public void LoadMap()
    {
        tileList.Add(new TileInfo
        {
            From = new Vector2(0, 0),
            To = new Vector2(100, 1)
        });

        tileList.Add(new TileInfo
        {
            From = new Vector2(99, 1),
            To = new Vector2(100, 100)
        });

        tileList.Add(new TileInfo
        {
            From = new Vector2(0, 99),
            To = new Vector2(100, 100)
        });

        tileList.Add(new TileInfo
        {
            From = new Vector2(0, 0),
            To = new Vector2(1, 100)
        });

        for(int i=0; i<20; i++)
        {
            for(int j=0; j<25; j++)
            {
                tileList.Add(new TileInfo
                {
                    From = new Vector2(i*5 + 4, j*4 + 4),
                    To = new Vector2(i * 5 + 4 + 3, j * 4 + 5)
                });
            }
        }
    }

    public void DrawMap()
    {
        var MapTiles = new GameObject("MapTiles");

        for (int i=0; i<tileList.Count; i++)
        {
            TileInfo currentTileInfo = tileList[i];
            GameObject tile = Instantiate(UnitTile, MapTiles.transform);
            SpriteRenderer tileRenderer = tile.GetComponent<SpriteRenderer>();
            BoxCollider2D tileCollider = tile.GetComponent<BoxCollider2D>();

            float centerX = (currentTileInfo.From.x + currentTileInfo.To.x) / 2;
            float centerY = (currentTileInfo.From.y + currentTileInfo.To.y) / 2;
            float sizeX = Mathf.Abs(currentTileInfo.To.x - currentTileInfo.From.x);
            float sizeY = Mathf.Abs(currentTileInfo.To.y - currentTileInfo.From.y);

            tile.transform.position = new Vector2(centerX, centerY);
            tileRenderer.size = new Vector2(sizeX, sizeY);
            tileCollider.size = new Vector2(sizeX, sizeY);
        }
    }
}
