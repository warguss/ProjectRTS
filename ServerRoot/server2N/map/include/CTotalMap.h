


class CTotalGameMap
{
	private:
		HasaMap<int, CUser> _allUser;
	
		HasahMap<int, CUser> *sector;

		bool _addSector(CUser* user);
		bool _deleteSector(CUser* user);	

	public:
		CGameLogic();
		~CGameLogic();
		
		void* MainLogic(void* inputClient);
		bool addClient(CUser* dclient);
		bool deleteClient(CUser* client);
		
};
