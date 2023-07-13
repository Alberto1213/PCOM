
Radu Alberto-Mihai 331CC
Tema2 Pcom 2023


   	Tema a avut ca scop crearea unui server, care prin intermediul conexiunilor TCP si UDP sa permita conecatarea unor abonati la server, 
prin intermediul socketilor.
Subscreiberii primesc un id la prima conecatare la server, ei putandu-se sa se conecteze la server cu store_forward setat pe 1 sau pe 0, 
diferenta fiind ca atunci cand este setat pe 1,mesajele sunt retinute si forwardate cat timp a fost online, 
iar cand este pe 0, mesajele abonatului sunt pierdute.

	Am folosit doua fisiere .cpp. in speta server.cpp si subscriber.cpp, astfel ca in server.cpp, am implementat functionalitati specifice unui server, 
anume conectarea, deconectarea, store and forwarding de mesaje, exit.
In fisierul subscriber.cpp sunt realizate cererile de printarea a mesajelor venite de la server.
De asemenea am folosit si un header utils.h in care am definit structurile necesare implementarii temei.

	Pentru a duce la capat aceasta tema m am folosit de cunostiintele dobandite in urma laboratoarelpor 5,6,7,8, fiind lab uri necesare pentru realizarea temei.
	


