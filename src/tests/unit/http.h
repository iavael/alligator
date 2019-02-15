#include "common/http.h"
void test_gen_http_query_1()
{
	char* query;
	query = gen_http_query(0, "/", "example.com", "testagent", NULL);
	cut_assert_equal_string(query, "GET / HTTP/1.1\nHost: example.com\nUser-Agent: testagent\n\n");
	free(query);
}

void test_gen_http_query_2()
{
	char* query;
	query = gen_http_query(0, "/api/v.1.3.4/query/?action=delete&button=pressed&dns=ok&request_id=oaqu7EoX", "EX.WWW.NEW.EXAMPLE.NET", "TeSTageNt/12.3.4 \\ (Test Ag. Ltd.) \\", NULL);
	cut_assert_equal_string(query, "GET /api/v.1.3.4/query/?action=delete&button=pressed&dns=ok&request_id=oaqu7EoX HTTP/1.1\nHost: EX.WWW.NEW.EXAMPLE.NET\nUser-Agent: TeSTageNt/12.3.4 \\ (Test Ag. Ltd.) \\\n\n");
	free(query);
}

void test_gen_http_query_3()
{
	char* query;
	query = gen_http_query(0, "/api/v.1.3.4/query/?action=delete&button=pressed&dns=ok&request_id=oaqu7EoX", "EX.WWW.NEW.EXAMPLE.NET", "TeSTageNt/12.3.4 \\ (Test Ag. Ltd.) \\", "dXNkcjpwYXNz");
	cut_assert_equal_string(query, "GET /api/v.1.3.4/query/?action=delete&button=pressed&dns=ok&request_id=oaqu7EoX HTTP/1.1\nHost: EX.WWW.NEW.EXAMPLE.NET\nUser-Agent: TeSTageNt/12.3.4 \\ (Test Ag. Ltd.) \\\nAuthorization: Basic dXNkcjpwYXNz\n\n");
	free(query);
}

void test_gen_http_query_4()
{
	char* query;
	query = gen_http_query(
	0,
	"/io2kei3nee3Ha3eMah9i/wai4Iiyae8aejie0ahpi/theefaethahM7xahweiv?theeviuja8thoo4theoGhoujeighua=aiCh1AoL1fuxui1phoQuaiX5Aeh7Th&aogh2hee7ahp7Dahch4iihoocaphu0=gohvoo0ahkaeM3uuy5Geefoh8Aat1f&AhCa8vuev2peCh4UajaoMeingahYueod4vei2Ohgh6iaWia8phu5eiyu6iet&eich5foneV9ePhi9hohng4ohbaik6ceo1hoo6aicee3eegh3UZiepeeraisughout4tee3ahng0aexee7ho3ohvohng4ao6tai8la4zeuFootoGooj0ahb9iexahNeecoom2Vah7Udophaiph5quiedeiwog8oothoo8xohb1aivodiechuwus9Iez2zah6ahnu6kauf=aeTheuz8fe0akuThiquoo4choe4ihe8ki3bu6ceeThi9ga5eeshae6ao5va7Er4ir0eich3phaesha5aum8redaiChieshe8uy8NaiCaqueiP3Ohng1hee0iyo9AiJ7Yuthochiezieph5aithaif0aebieghaquoe5phei6Sei9iechohlaez5aighaiPhe2Uyoos0h&eibusashaeBaih5xee6xah9cuSho5Igahx6chahchaizei1ohk9hie2nei9iephai5ieV7quezaupeitashuqueiM5iucoofe7eochoh4Ti5aewu9ahgh1iePieth1bai8chae5aef6Oocaip9IZ5eiRohceichoh1kohvooSuChiphieyohrah1ein3ohf7ich8ieJi=yei3echaimohch0ees9ThaiC5feimeipaesad5Shucoec8ax8UweTi6iejouch9AhngohPhohyahah9ieQuaf7ahlohdu3uPh3oube0daeph5goofa9wifaokohzaiv0zae2aemei5IshoJoi8iphoo3le4zeeso0ImogheipePha5queejo4Reeph9chuti5aiWaeh8&zexeiveeXee5phaexoaphooyohn1sahchuxieguqu3eixohJoone8Phaid6engi6Foox5quuof7vohkoodoosog7OhJa3lu2Iequoh9xeiphahreoshiGh1xogh9pil8ahsaighae9saeNgae9En5Ieroi9uso7theeShi8aiTee2ZahdaoSh6fi2soo8fuvoorijeor=augh9eeNgoorohghe8thaeFeiseip5Noocoh6phai9Ohm2ESeokaefi2thoo7dai7iQuouHoma5Eit8reeraiSh9eicalahwoozai9Eezaicohg4Caejira3axouvaCh7ahchutheith3sheeyiphune1yee5Zu6ahVeepooh7veeph4sahcah2aiw1kaik2Veithush&vai0mai5jaeGh5JooV4oj5sai0woF2Jeedagei2oquooshie9Aiteirei1ajaJai9eth5ahCah4wuRing8yeizuashaeyohc3riefu5Jev7pheophoachofohchohMaiJuwoht4hiedaif0Ur2oosh3aegoot1dai2die1aej5Thayoo7iquah3qua8Choufie2Rie5r=TooSho0eiliceijooh3Go3doojeegh5shohch7phex1aijulefae3zoy6eemaoh2Cheephaethaedohgheve8vueNohpheji7aiXoodohh6Yahg3AeKiech5avuiz8jiegooj4ONaeGhooreilaeBai6ahG2Tee5zeeG0ree7Chee6aereeGooKooYae3bo3ree3quae",
	"eivatah4laiqu4sai7esh8eMaoPhah5Ahso9zobia9quohSeechaiph7tah1goh3Zii9hahc9wahx4ieS8paiVie2OoZooveex2irei0Fie1xee0ioV2ka1ahh8beD0siec3rizahv0EeVi7paep5choceejei7kiek0ij7alequeecahquaid1Lee9eewi3aeg1aixieRaid1yeethopha4ech3Choowee5eingaewaexu0bievaithais.org",
	"eeF6aes3ahre9aideisheikua9phoh1oox4aiyoghu6zaiph8ang1reyiph1Iyo7gohchae3osh4caizie3Ono0baQu4nii8nuo1thahrahza4ziexaenaep3Eiphaixao0eeses8Dohfaexee8ooriequacoph2ooxieLoochahquahtair6chooweishoo4ieziucahx7ohR6oomaJ3eixee7Yovu0aekeech4sei0aigh8Ax0ohfu7ahf3me",
	"cGhhaWtvb2Z1bWVldnV1N0VpbGVpdmVlMHVkaWVzaGFlOUVXaWU3SXVHM1RvbzhhYjRhZWowWG9vczNub2g1T28wcGhlNG9oZ2hlaWtlaTJHYWUyeWl1WWllOEFlcDF3YTNhZXlhaW0zYWhtNWRvb05nYXF1ZWk4YWlxdWFpVGgzdWhhaGJhaW4xb2hobzByZWk4Z2FobmdlZTNoaWU4YWVLaTlFb2g0ZWlkOGllbWFlbmdlZTluaXhvZXF1ZWVodTZvb2hheTBoYWg1bGFpdmFvdjFvaTNrdXZhZXJvb3dhaTJpZWtvbzhNZWU3dWV5ZWk5ZWlYb296M3d1a29o"
	);

	cut_assert_equal_string(query, "GET /io2kei3nee3Ha3eMah9i/wai4Iiyae8aejie0ahpi/theefaethahM7xahweiv?theeviuja8thoo4theoGhoujeighua=aiCh1AoL1fuxui1phoQuaiX5Aeh7Th&aogh2hee7ahp7Dahch4iihoocaphu0=gohvoo0ahkaeM3uuy5Geefoh8Aat1f&AhCa8vuev2peCh4UajaoMeingahYueod4vei2Ohgh6iaWia8phu5eiyu6iet&eich5foneV9ePhi9hohng4ohbaik6ceo1hoo6aicee3eegh3UZiepeeraisughout4tee3ahng0aexee7ho3ohvohng4ao6tai8la4zeuFootoGooj0ahb9iexahNeecoom2Vah7Udophaiph5quiedeiwog8oothoo8xohb1aivodiechuwus9Iez2zah6ahnu6kauf=aeTheuz8fe0akuThiquoo4choe4ihe8ki3bu6ceeThi9ga5eeshae6ao5va7Er4ir0eich3phaesha5aum8redaiChieshe8uy8NaiCaqueiP3Ohng1hee0iyo9AiJ7Yuthochiezieph5aithaif0aebieghaquoe5phei6Sei9iechohlaez5aighaiPhe2Uyoos0h&eibusashaeBaih5xee6xah9cuSho5Igahx6chahchaizei1ohk9hie2nei9iephai5ieV7quezaupeitashuqueiM5iucoofe7eochoh4Ti5aewu9ahgh1iePieth1bai8chae5aef6Oocaip9IZ5eiRohceichoh1kohvooSuChiphieyohrah1ein3ohf7ich8ieJi=yei3echaimohch0ees9ThaiC5feimeipaesad5Shucoec8ax8UweTi6iejouch9AhngohPhohyahah9ieQuaf7ahlohdu3uPh3oube0daeph5goofa9wifaokohzaiv0zae2aemei5IshoJoi8iphoo3le4zeeso0ImogheipePha5queejo4Reeph9chuti5aiWaeh8&zexeiveeXee5phaexoaphooyohn1sahchuxieguqu3eixohJoone8Phaid6engi6Foox5quuof7vohkoodoosog7OhJa3lu2Iequoh9xeiphahreoshiGh1xogh9pil8ahsaighae9saeNgae9En5Ieroi9uso7theeShi8aiTee2ZahdaoSh6fi2soo8fuvoorijeor=augh9eeNgoorohghe8thaeFeiseip5Noocoh6phai9Ohm2ESeokaefi2thoo7dai7iQuouHoma5Eit8reeraiSh9eicalahwoozai9Eezaicohg4Caejira3axouvaCh7ahchutheith3sheeyiphune1yee5Zu6ahVeepooh7veeph4sahcah2aiw1kaik2Veithush&vai0mai5jaeGh5JooV4oj5sai0woF2Jeedagei2oquooshie9Aiteirei1ajaJai9eth5ahCah4wuRing8yeizuashaeyohc3riefu5Jev7pheophoachofohchohMaiJuwoht4hiedaif0Ur2oosh3aegoot1dai2die1aej5Thayoo7iquah3qua8Choufie2Rie5r=TooSho0eiliceijooh3Go3doojeegh5shohch7phex1aijulefae3zoy6eemaoh2Cheephaethaedohgheve8vueNohpheji7aiXoodohh6Yahg3AeKiech5avuiz8jiegooj4ONaeGhooreilaeBai6ahG2Tee5zeeG0ree7Chee6aereeGooKooYae3bo3ree3quae HTTP/1.1\nHost: eivatah4laiqu4sai7esh8eMaoPhah5Ahso9zobia9quohSeechaiph7tah1goh3Zii9hahc9wahx4ieS8paiVie2OoZooveex2irei0Fie1xee0ioV2ka1ahh8beD0siec3rizahv0EeVi7paep5choceejei7kiek0ij7alequeecahquaid1Lee9eewi3aeg1aixieRaid1yeethopha4ech3Choowee5eingaewaexu0bievaithais.org\nUser-Agent: eeF6aes3ahre9aideisheikua9phoh1oox4aiyoghu6zaiph8ang1reyiph1Iyo7gohchae3osh4caizie3Ono0baQu4nii8nuo1thahrahza4ziexaenaep3Eiphaixao0eeses8Dohfaexee8ooriequacoph2ooxieLoochahquahtair6chooweishoo4ieziucahx7ohR6oomaJ3eixee7Yovu0aekeech4sei0aigh8Ax0ohfu7ahf3me\nAuthorization: Basic cGhhaWtvb2Z1bWVldnV1N0VpbGVpdmVlMHVkaWVzaGFlOUVXaWU3SXVHM1RvbzhhYjRhZWowWG9vczNub2g1T28wcGhlNG9oZ2hlaWtlaTJHYWUyeWl1WWllOEFlcDF3YTNhZXlhaW0zYWhtNWRvb05nYXF1ZWk4YWlxdWFpVGgzdWhhaGJhaW4xb2hobzByZWk4Z2FobmdlZTNoaWU4YWVLaTlFb2g0ZWlkOGllbWFlbmdlZTluaXhvZXF1ZWVodTZvb2hheTBoYWg1bGFpdmFvdjFvaTNrdXZhZXJvb3dhaTJpZWtvbzhNZWU3dWV5ZWk5ZWlYb296M3d1a29o\n\n");
	free(query);
}