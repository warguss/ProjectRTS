using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ExplosionDamage : MonoBehaviour
{

    WeaponId explosionType;
    float Damage = 0f;
    float ExplosionDuration = 0.5f;
    float ExplosionSize = 3f;
    float ExplosionImpact = 1f;
    int OwnerId = -1;

    bool ExplosionStarted = false;
    float ExplosionElasped = 0f;
    SpriteRenderer spriteRenderer;
    BoxCollider2D boxCollider2D;

    public void Initialize(WeaponId inType, float inDamage, float inExplosionDuration, float inExplosionSize, float inImpact, int inOwnerId)
    {
        explosionType = inType;
        Damage = inDamage;
        ExplosionDuration = inExplosionDuration;
        ExplosionSize = inExplosionSize;
        ExplosionImpact = inImpact;
        OwnerId = inOwnerId;
    }

    public void DoExplotion()
    {
        spriteRenderer.size = new Vector2(ExplosionSize, ExplosionSize);
        boxCollider2D.size = new Vector2(ExplosionSize, ExplosionSize);
        ExplosionStarted = true;
    }

    private void Awake()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();
        boxCollider2D = GetComponent<BoxCollider2D>();
    }

    // Use this for initialization
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {
        if (ExplosionStarted)
        {
            ExplosionElasped += Time.deltaTime;
            float alpha = 1 - (ExplosionElasped / ExplosionDuration);
            Color currentColor = spriteRenderer.color;
            spriteRenderer.color = new Color(currentColor.r, currentColor.g, currentColor.b, alpha);
        }
        if (ExplosionElasped > ExplosionDuration)
            Destroy(gameObject);
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (ExplosionStarted)
        {
            if (other.tag == "Player")
            {
                MainCharacter targetPlayer = other.gameObject.GetComponent<MainCharacter>();
                if (targetPlayer.OwnerId != OwnerId)
                {
                    if (targetPlayer.IsLocalPlayer)
                    {
                        Vector2 ExplotionPosition = new Vector2(gameObject.transform.position.x, gameObject.transform.position.y);
                        Vector2 hitVector = targetPlayer.GetCurrentPosition() - ExplotionPosition;
                        Vector2 hitVectorNormalized = hitVector.normalized;

                        HitInfo hitInfo = new HitInfo();
                        hitInfo.Damage = Damage;
                        hitInfo.HitType = explosionType;
                        hitInfo.ImpactX = hitVectorNormalized.x * ExplosionImpact;
                        hitInfo.ImpactY = hitVectorNormalized.y * ExplosionImpact;

                        targetPlayer.GetHit(OwnerId, hitInfo);
                    }
                }
                else
                {
                }
            }
        }
    }
}
