using UnityEngine;
using System.Collections;

public class CameraMove : MonoBehaviour {

    float MAX_MOVE_SPEED = 0.1f;

    public GameObject MainCamera;
    public float cameraDelay = 0.3f;

    GameObject Target;

    Vector3 srcPos;
    Vector3 targetPos;
    float accumulatedMoveTime = 0f;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
        if (Target)
        {
            Vector3 prevTargetPos = targetPos;
            targetPos = Target.transform.position;
            if (prevTargetPos != targetPos)
            {
                srcPos = MainCamera.transform.position;
                accumulatedMoveTime = 0f;
            }

            Vector3 currentCameraPos = MainCamera.transform.position;

            //MainCamera.transform.position = new Vector3(myPosition.x, myPosition.y, -10);
            Vector3 newCameraPos = Vector3.Lerp(srcPos, targetPos, Mathf.Sqrt(accumulatedMoveTime / cameraDelay));
            newCameraPos.z = -10;

            MainCamera.transform.position = newCameraPos;

            accumulatedMoveTime += Time.deltaTime;
        }
	}

    public void SetTarget(GameObject inTarget)
    {
        Target = inTarget;
        srcPos = Target.transform.position;
    }
}
