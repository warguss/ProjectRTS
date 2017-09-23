using UnityEngine;
using System.Collections;
using RTS;

public enum CharacterStatus
{
	Neutral,
    Moving,
    inAir,
    Attacking,
	Attacked,
}

public class MainCharacter : MonoBehaviour
{
	public float MaxMoveSpeed = 100;
	public float MoveForce = 100;
	public float JumpForce =0.01f;

    public GameObject bulletPrefab;

    private Rigidbody2D rb2d;

    private CharacterStatus status = CharacterStatus.Neutral;
    private int hitRecovery = 0;

    private int posX;
    private int posY;
    //private int speedX;
    //private int speedY;

    private PlayerInput currentInput;

	// Use this for initialization
	void Start()
	{
		rb2d = GetComponent<Rigidbody2D>();
        PosInt gamePos = Utils.RbPosToGamePos(rb2d.position.x, rb2d.position.y);
        posX = gamePos.x;
        posY = gamePos.y;
	}

    void SetPosition(int posX, int posY)
    {
        PosFloat Rb2dPos = Utils.GamePosToRbPos(posX, posY);
        rb2d.position.Set(Rb2dPos.x, Rb2dPos.y);
    }
	
	// Update is called once per frame
	void Update()
    {
        Debug.Log(posX + ", " + posY);
	}

	void FixedUpdate()
	{
        if (currentInput.jump && (status == CharacterStatus.Neutral || status == CharacterStatus.Moving))
            Jump();

        if (currentInput.fire && (status == CharacterStatus.Neutral || status == CharacterStatus.Moving))
            Fire();
        
        if (currentInput.left && status != CharacterStatus.Attacked)
            MoveLeft();
        
        else if (currentInput.right && status != CharacterStatus.Attacked)
            MoveRight();
        
        PosInt gamePos = Utils.RbPosToGamePos(rb2d.position.x, rb2d.position.y);
        posX = gamePos.x;
        posY = gamePos.y;
	}

    public void SetInput(PlayerAction action)
    {
        switch (action)
        {
            case PlayerAction.Stop:
                currentInput.left = false;
                currentInput.right = false;
                break;
            case PlayerAction.Left:
                currentInput.left = true;
                break;
            case PlayerAction.Right:
                currentInput.right = true;
                break;
            case PlayerAction.Jump:
                currentInput.jump = true;
                break;
        }
    }

    void MoveLeft()
    {
        if (-1 * rb2d.velocity.x < MaxMoveSpeed)
            rb2d.AddForce(Vector2.right * -1 * MoveForce);
        
        if (Mathf.Abs (rb2d.velocity.x) > MaxMoveSpeed)
            rb2d.velocity = new Vector2(Mathf.Sign (rb2d.velocity.x) * MaxMoveSpeed, rb2d.velocity.y);
    }

    void MoveRight()
    {
        if (1 * rb2d.velocity.x < MaxMoveSpeed)
            rb2d.AddForce(Vector2.right * 1 * MoveForce);

        if (Mathf.Abs (rb2d.velocity.x) > MaxMoveSpeed)
            rb2d.velocity = new Vector2(Mathf.Sign (rb2d.velocity.x) * MaxMoveSpeed, rb2d.velocity.y);
    }

    void Jump()
    {
        //rb2d.AddForce(Vector2.up*JumpForce);
        rb2d.velocity = new Vector2(rb2d.velocity.x, JumpForce);
        currentInput.jump = false;
    }

    void Fire()
    {
        Instantiate(bulletPrefab, gameObject.transform.position, new Quaternion());
    }

    void GetHit(int damage, int hitRecovery)
    {
        
    }

    //void Networkinterpolation(float posX, float posY, float speedX, float speedY, CharacterStatus status)
    //{
    //}
}
