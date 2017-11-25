


#include <iostrem.h>

class CUser
{
	private:
		int32_t uniqueNo;
		int32_t pX;
		int32_t pY;
		
		int32_t walkSpeed;
		int32_t weaponType;
			
	public:
		CUser();
		~CUser();

		int setWeapon();
		int getWalk();
};



