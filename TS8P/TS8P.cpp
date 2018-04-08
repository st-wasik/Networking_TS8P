#include "SFML/Network.hpp"
#include "iostream"
#include "ctime"
#include "sstream"
#include <limits>
#include <bitset>

std::string getLocalTime()
{
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	auto r = std::string(asctime(timeinfo));
	r.pop_back();
	return r;
}

template <typename T>
std::string IntToString(const T& t)
{
	std::ostringstream r; r << t;
	return r.str();
}

template <typename T>
T StringTo(std::string s)
{
	bool minus = 0;
	T result = 0;
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == 45)
		{
			minus = 1;
		}
		else
		{
			result += (s[i] - 48);
			if (i != s.size() - 1) result *= 10;
		}
	}
	if (minus) return result*(-1);
	return result;
}

// pakiet protokołu - przechowuje dane
class TS8P final
{
public:
	struct Status
	{
		static const std::string ustawID;
		static const std::string wykonaj;
		static const std::string wynik;
		static const std::string zakoncz;
		static const std::string przepelnienie;
		static const std::string error;
	};

	struct Operacja
	{
		static const std::string dodaj;
		static const std::string odejmij;
		static const std::string mnozenie;
		static const std::string dzielenie;

		static const std::string reszta_z_dzielenia;
		static const std::string potega;
		static const std::string suma_bitowa;
		static const std::string XOR;
	};
	/////////////////////////////////

	TS8P()
		: operacja{ "Operacja: ","" }, status{ "Status: ","" },
		identyfikator{ "Identyfikator: ",0 }, czas{ "Czas: ","" },
		liczba_operandow{ "Liczba_operandow: ", 0 }, operand{ "Operand: ", std::vector<sf::Int32>() },
		wynik{ "Wynik: ", "" }
	{
	}

	//np. "operacja","dodaj"
	std::pair<std::string, std::string> operacja;
	//np. "status","wykonaj"
	std::pair<std::string, std::string> status;
	//np. "identyfikator",123
	std::pair<std::string, sf::Uint16> identyfikator;
	std::pair<std::string, std::string> czas;
	std::pair<std::string, sf::Uint16> liczba_operandow;
	std::pair<std::string, std::vector<sf::Int32>> operand;
	std::pair<std::string, std::string> wynik;

	// przygotowanie pakietu do wyświetlenia na ekranie
	operator std::string()
	{
		std::ostringstream result;
		result << "* * * * * KOMUNIKAT--TS8P * * * * *" << std::endl;
		result << identyfikator.first << identyfikator.second << std::endl;
		result << czas.first << czas.second << std::endl;
		result << operacja.first << operacja.second << std::endl;
		result << liczba_operandow.first << liczba_operandow.second << std::endl;
		result << operand.first;
		for (int i = 0; i < liczba_operandow.second; i++) { result << operand.second[i] << " "; }
		if (liczba_operandow.second == 0) { result << ""; } result << std::endl;
		result << status.first << status.second << std::endl;
		result << wynik.first << wynik.second << std::endl;
		result << "* * * * * * * * * * * * * * * * * *";
		return result.str();
	}

	// zamiana pakietu na tablicę char
	char* toCharArray()
	{
		std::string result;
		result += czas.first;
		result += czas.second + ";";

		result += operacja.first;
		result += operacja.second + ";";

		result += status.first;
		result += status.second + ";";

		result += identyfikator.first;
		result += IntToString(identyfikator.second) + ";";

		result += liczba_operandow.first;
		result += IntToString(liczba_operandow.second) + ";";

		result += operand.first;
		if (liczba_operandow.second)
			for (int i = 0; i < liczba_operandow.second; i++)
			{
				result += IntToString(operand.second[i]) + ";";
			}
		else
			result += ";";

		result += wynik.first;
		result += wynik.second + ";";
		char * res = new char[result.size() + 1];
		std::strcpy(res, result.c_str());
		return res;
	}

	// wypełnienie pakietu danymi zawartymi w tablicy char
	void fromCharArray(char* arr, std::size_t size)
	{
		*this = TS8P();
		std::string result;
		for (int i = 0; i < size; i++)
		{
			result += arr[i];
		}

		result.erase(0, 6);
		czas.second = result.substr(0, 24);
		result.erase(0, 25);

		operacja.second = toNextSemi(result);
		status.second = toNextSemi(result);
		identyfikator.second = StringTo<sf::Uint16>(toNextSemi(result));
		liczba_operandow.second = StringTo<sf::Uint16>(toNextSemi(result));
		if (liczba_operandow.second == 0)
			toNextSemi(result);
		for (int i = 0; i < liczba_operandow.second; i++)
		{
			operand.second.push_back(StringTo<sf::Int32>(toNextSemi(result)));
		}
		wynik.second = toNextSemi(result);
	}

	// służy do wycinania informacji z odebranych danych
	// zwraca informację zawartą między znakami ':' i ';'
	static std::string toNextSemi(std::string& s)
	{
		std::string result;
		while (s.size() > 0)
		{
			if (s[0] != ';')
			{
				if (s[0] == ' ')
				{
					s.erase(0, 1);
				}
				else
				{
					result += s[0];
					if (s[0] == ':')
						result.clear();
					s.erase(0, 1);
				}
			}
			else
			{
				s.erase(0, 1);
				return result;
			}
		}
		return result;
	}

	sf::Uint32 size()
	{
		std::string result;
		result += operacja.first;
		result += operacja.second + ";";

		result += status.first;
		result += status.second + ";";

		result += identyfikator.first;
		result += IntToString(identyfikator.second) + ";";

		result += czas.first;
		result += czas.second + ";";

		result += liczba_operandow.first;
		result += IntToString(liczba_operandow.second) + ";";

		result += operand.first;

		if (liczba_operandow.second)
			for (int i = 0; i < liczba_operandow.second; i++)
			{
				result += IntToString(operand.second[i]) + ";";
			}
		else
			result += ";";

		result += wynik.first;
		result += IntToString(wynik.second) + ";";
		return result.size();
	}
};

const std::string TS8P::Operacja::dodaj = "dodaj";
const std::string TS8P::Operacja::odejmij = "odejmij";
const std::string TS8P::Operacja::mnozenie = "mnozenie";
const std::string TS8P::Operacja::dzielenie = "dzielenie";
const std::string TS8P::Operacja::reszta_z_dzielenia = "reszta";
const std::string TS8P::Operacja::potega = "potega";
const std::string TS8P::Operacja::suma_bitowa = "bitOR";
const std::string TS8P::Operacja::XOR = "bitXOR";

const std::string TS8P::Status::ustawID = "ustawID";
const std::string TS8P::Status::wykonaj = "wykonaj";
const std::string TS8P::Status::wynik = "wynik";
const std::string TS8P::Status::zakoncz = "zakoncz";
const std::string TS8P::Status::przepelnienie = "przepelnienie";
const std::string TS8P::Status::error = "blad";

// do ustawiania drugiej wartości std::pair
template <typename T, typename R>
void operator<<(std::pair<T, R>& p, const R& e)
{
	p.second = e;
}

// do pobierania drugiej wartości std::pair
template <typename T, typename R>
void operator<<(R& e, std::pair<T, R>& p)
{
	e = p.second;
}

// program serwera
class server final
{
public:
	server() : serverPort(28000)
	{
	}
	sf::TcpSocket socket;
	sf::TcpListener listener;
	unsigned short serverPort;
	sf::Uint16 clientID;

	// słyży wysłania pakietu do klienta
	void send(TS8P& t)
	{
		t.identyfikator << clientID;
		t.czas << getLocalTime();
		char* SEND = t.toCharArray();
		// wysłanie danych do klienta oraz obsługa ewentualnych błędów
		auto status = socket.send(SEND, t.size());
		if (status != sf::Socket::Done)
		{
			std::cout << "Send-error code: " << status << std::endl;
		}
		delete[] SEND;
		std::cout << "\nWYŚLIJ:\n" << t.operator std::string() << std::endl;
	}

	// odebranie danych od klienta
	void receive(TS8P& t)
	{
		char* RECV = new char[4096];
		std::size_t received;
		// odebranie danych (4096 - rozmiar bufora)
		auto status = socket.receive(RECV, 4096, received);
		if (status != sf::Socket::Done)
		{
			std::cout << "Receive-error code: " << status << std::endl;
			if (status == sf::Socket::Disconnected)
			{
				std::cout << "Utracono połączenie z serwerem. Exit..." << std::endl;
				sf::sleep(sf::seconds(3));
				exit(-1);
			}
		}
		else
		{
			// wypełnienie pakietu t odebranymi danymi
			t.fromCharArray(RECV, received);
			std::cout << "\nODBIERZ:\n" << t.operator std::string() << std::endl;
		}
		delete[] RECV;
	}

	// nasłuchiwanie, połączenie klienta
	void listen()
	{
		srand(time(NULL));
		system("cls");
		std::cout << "TS8P Server " << sf::IpAddress::getLocalAddress() << ":" << serverPort << std::endl;
		std::cout << "Oczekiwanie na klienta" << std::endl;
		// nasłuchiwanie
		if (listener.listen(serverPort) != sf::Socket::Done)
		{
			std::cout << "Listen fail. Exit..." << std::endl;
			sf::sleep(sf::seconds(3));
			exit(-1);
		}
		// połączenie klienta
		if (listener.accept(socket) != sf::Socket::Done)
		{
			std::cout << "Accept fail. Exit..." << std::endl;
			sf::sleep(sf::seconds(3));
			exit(-1);
		}
		else
		{
			listener.close();
			std::cout << "Połączono z klientem " << socket.getRemoteAddress() << ":" << socket.getRemotePort() << std::endl;
		}
	}

	void run()
	{
		listen();

		TS8P t;
		t.status << TS8P::Status::ustawID;
		clientID = rand() % 254 + 1; // wylosowanie ID
		send(t);

		do
		{
			// odebranie danych oraz obsługa poleceń
			TS8P t;
			receive(t);
			std::string status, operacja;
			status << t.status; operacja << t.operacja;

			if (status == TS8P::Status::wykonaj)
			{
				if (operacja == TS8P::Operacja::dodaj)
				{
					dodaj(t);
				}
				else if (operacja == TS8P::Operacja::odejmij)
				{
					odejmij(t);
				}
				else if (operacja == TS8P::Operacja::mnozenie)
				{
					monozenie(t);
				}
				else if (operacja == TS8P::Operacja::dzielenie)
				{
					dzielenie(t);
				}
				else if (operacja == TS8P::Operacja::reszta_z_dzielenia)
				{
					reszta(t);
				}
				else if (operacja == TS8P::Operacja::potega)
				{
					potega(t);
				}
				else if (operacja == TS8P::Operacja::suma_bitowa)
				{
					suma(t);
				}
				else if (operacja == TS8P::Operacja::XOR)
				{
					XOR(t);
				}
				else
				{
					std::cout << "Operacja wskazana przez klienta nie została rozpoznana." << std::endl;
				}
			}
			else if (status == TS8P::Status::zakoncz)
			{
				std::cout << "Klient zażądał zakończenia połączenia" << std::endl;
				// wysłanie potwierdzenia zakończenia sesji
				TS8P t;
				t.status << TS8P::Status::zakoncz;
				send(t);
				// rozłącznie
				socket.disconnect();
				std::cout << "Klient zakończył połączenie. Exit..." << std::endl;
				sf::sleep(sf::seconds(5));
				break;
			}
		} while (1);
	}

	// funkcje wykonujące operacje matematyczne
	// przy wykryciu błędu lub przepełnienia wysyłany jest pakiet o tym informujący
	void dodaj(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;
		sf::Int64 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			wynik += sf::Int64(vect[i]);
			if (sf::Int64(wynik) != sf::Int32(wynik))
			{
				std::cout << "WYKRYTO PRZEPEŁNIENIE." << std::endl;
				TS8P zwrot;
				zwrot.status << TS8P::Status::przepelnienie;
				send(zwrot);
				return;
			}
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		sf::Int32 res = wynik;
		zwrot.wynik.second = IntToString(res);
		send(zwrot);

	}
	void odejmij(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int64 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			wynik -= vect[i];
			if (sf::Int64(wynik) != sf::Int32(wynik))
			{
				std::cout << "WYKRYTO PRZEPEŁNIENIE." << std::endl;
				TS8P zwrot;
				zwrot.status << TS8P::Status::przepelnienie;
				send(zwrot);
				return;
			}
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		sf::Int32 res = wynik;
		zwrot.wynik.second = IntToString(res);
		send(zwrot);

	}
	void monozenie(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int64 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			wynik *= vect[i];

			if (sf::Int64(wynik) != sf::Int32(wynik))
			{
				std::cout << "WYKRYTO PRZEPEŁNIENIE." << std::endl;
				TS8P zwrot;
				zwrot.status << TS8P::Status::przepelnienie;
				send(zwrot);
			}
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		sf::Int32 res = wynik;
		zwrot.wynik.second = IntToString(res);
		send(zwrot);

	}
	void dzielenie(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int32 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			if (vect[i] == 0)
			{
				std::cout << "DZIELENIE PRZEZ ZERO" << std::endl;
				TS8P zwrot;
				zwrot.status << TS8P::Status::error;
				send(zwrot);
				return;
			}

			wynik /= vect[i];
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		zwrot.wynik.second = IntToString(wynik);
		send(zwrot);
	}
	void reszta(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int32 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			if (vect[i] == 0)
			{
				std::cout << "DZIELENIE PRZEZ ZERO" << std::endl;
				TS8P zwrot;
				zwrot.status << TS8P::Status::error;
				send(zwrot);
				return;
			}
			wynik %= vect[i];
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		zwrot.wynik.second = IntToString(wynik);
		send(zwrot);
	}
	void potega(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int64 wynik = vect[0];

		// obliczanie wykładnika 
		sf::Int32 b = vect[1];
		for (int i = 2; i < vect.size(); i++)
		{
			b *= vect[i];
		}
		std::cout << "(" << vect[0] << "^" << (b) << ")" << std::endl;

		// obsługa wariantów potęgowania
		if (b == 0)
		{
			wynik = 1;
		}
		else if (b < 0)
		{
			// protokół umożliwia jedynie wykładniki >=0
			std::cout << "WYKLADNIK < 0." << std::endl;
			TS8P zwrot;
			zwrot.status << TS8P::Status::error;
			send(zwrot);
			return;
		}
		else
		{
			for (int j = 1; j < b; j++)
			{
				wynik *= vect[0];
				if (sf::Int64(wynik) != sf::Int32(wynik))
				{
					std::cout << "WYKRYTO PRZEPEŁNIENIE." << std::endl;
					TS8P zwrot;
					zwrot.status << TS8P::Status::przepelnienie;
					send(zwrot);
					return;
				}
			}
		}


		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		sf::Int32 res = wynik;
		zwrot.wynik.second = IntToString(res);
		send(zwrot);

	}
	void suma(TS8P& t)
	{
		//suma bitowa
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int32 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			wynik |= vect[i];
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		zwrot.wynik.second = IntToString(wynik);
		send(zwrot);
	}
	void XOR(TS8P& t)
	{
		std::vector<sf::Int32>& vect = t.operand.second;
		if (!vect.size()) return;

		sf::Int32 wynik = vect[0];

		for (int i = 1; i < vect.size(); i++)
		{
			wynik ^= vect[i];
		}

		TS8P zwrot;
		zwrot.status << TS8P::Status::wynik;
		zwrot.wynik.second = IntToString(wynik);
		send(zwrot);
	}
};

// program klienta
class client final
{
public:
	client()
	{}

	sf::TcpSocket socket;
	sf::Uint16 clientID;

	void send(TS8P& t)
	{
		t.identyfikator << clientID;
		t.czas << getLocalTime();
		char* SEND = t.toCharArray();
		auto status = socket.send(SEND, t.size());
		if (status != sf::Socket::Done)
		{
			std::cout << "Send-error code: " << status << std::endl;
		}
		delete[] SEND;
	}

	void receive(TS8P& t)
	{
		char* RECV = new char[4096];
		std::size_t received;
		auto status = socket.receive(RECV, 4096, received);
		if (status != sf::Socket::Done)
		{
			std::cout << "Receive-error code: " << status << std::endl;
			if (status == sf::Socket::Disconnected)
			{
				std::cout << "Utracono połączenie z serwerem. Exit..." << std::endl;
				sf::sleep(sf::seconds(3));
				exit(-1);
			}
		}
		else
		{
			t.fromCharArray(RECV, received);
		}
	}

	// wyświetlanie interfejsu użytkownika
	void interface()
	{
		std::cout << std::string(40, '*') << std::endl;
		unsigned int wybor;
		do {
			std::cout  << "\n 0- Exit\n 1- Dodaj\n 2- Odejmij\n 3- Mnożenie\n 4- Dzielenie\n 5- Reszta z dzielenia\n 6- Potega ((n^x)^y)^z...\n 7- Suma bitowa\n 8- Bit XOR \n>" << std::flush;
			std::cin >> wybor;
			system("CLS");
			std::cout << std::string(40, '*') << "\n" << "int32 max: " << std::numeric_limits<sf::Int32>::max() << "   int32 min:" << std::numeric_limits<sf::Int32>::min() <<"\n"<< std::string(40, '*') << std::endl;

		} while (wybor < 0 || wybor > 8);

		switch (wybor)
		{
		case 0:
			close();
			break;
		case 1:
			dodaj();
			break;
		case 2:
			odejmij();
			break;
		case 3:
			mnozenie();
			break;
		case 4:
			dzielenie();
			break;
		case 5:
			reszta();
			break;
		case 6:
			potega();
			break;
		case 7:
			suma();
			break;
		case 8:
			XOR();
			break;
		}

	}

	// odebranie i obsułga wyniku
	void result()
	{
		TS8P t;
		receive(t);

		if (t.status.second == TS8P::Status::wynik)
		{
			std::cout << std::string(40, '-') << "\n" << "Serwer zwrócił wynik: " << t.wynik.second << "\n" << std::string(40, '-') << std::endl;
		}
		else if (t.status.second == TS8P::Status::przepelnienie)
		{
			std::cout << std::string(40, '-') << "\n"<< "Wynik jest zbyt długi, by go prawidłowo przesłać." << "\n" << std::string(40, '-') << std::endl;
		}
		else if (t.status.second == TS8P::Status::zakoncz)
		{
			std::cout << "Serwer potwierdził zakończenie sesji. Exit...";
			sf::sleep(sf::seconds(5));
			exit(0);
		}
		else if (t.status.second == TS8P::Status::error)
		{
			std::cout << std::string(40, '-') << "\n" << "Wystąpił błąd podczas obliczeń." << "\n" << std::string(40, '-') << std::endl;
		}
	}

	void run()
	{
		connect();
		std::cout << "TS8P Client " << sf::IpAddress::getLocalAddress() << ":" << socket.getLocalPort() << std::endl;
		receiveID();

		while (1)
		{
			interface();
			result();
		}
	}

	// połączenie z serwerem
	void connect()
	{
		std::string address;
		std::cout << "Podaj adres serwera: ";
		std::cin >> address;

		if (socket.connect(sf::IpAddress(address), 28000) != sf::Socket::Done)
		{
			std::cout << "Connect fail. Exit..." << std::endl;
			sf::sleep(sf::seconds(3));
			exit(-1);
		}
		system("cls");
	}

	// odebranie pierwszego pakietu zawierającego ID
	void receiveID()
	{
		TS8P t;
		receive(t);
		clientID << t.identyfikator;
		std::cout << "Identyfikator sesji: " << clientID << std::endl;
	}

	// zamknięcie połączenia
	void close()
	{
		TS8P t;
		std::cout << "Zamykanie połączenia." << std::endl;
		t.status << TS8P::Status::zakoncz;
		send(t);
	}

	// wyczytywanie danych - wykorzystywane w funkcjach odepowiedzialnych za przygotowanie pakietu z żądaniem usługi
	TS8P wczytaj()
	{
		TS8P result;
		sf::Uint16 wybor;
		do
		{
			std::cout << "Podaj liczbę operandów: ";
			std::cin >> wybor;
			if (wybor < 2) std::cout << "Należy podać przyjnamniej 2 operandy." << std::endl;
		} while (wybor < 2);

		for (int i = 0; i < wybor; i++)
		{
			sf::Int32 op;
			std::cout << "Podaj operand nr " << i + 1 << ": ";

			if (!(std::cin >> op))
			{
				std::cout << "Błąd wczytywania danych." << std::endl;
				std::cin.clear();
				result.operand.second.clear();
				result.operand.second.resize(2);
				result.liczba_operandow.second = result.operand.second.size();
				return result;
			}

			result.operand.second.push_back(op);
		}
		result.liczba_operandow.second = wybor;
		return result;
	}


	// funkcjae odepowiedzialnye za przygotowanie pakietu z żądaniem usługi
	void dodaj()
	{
		std::cout << "Operacja: dodawanie" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::dodaj;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
	void odejmij()
	{
		std::cout << "Operacja: odejmowanie" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::odejmij;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
	void mnozenie()
	{
		std::cout << "Operacja: mnożenie" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::mnozenie;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
	void dzielenie()
	{
		std::cout << "Operacja: dzielenie" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::dzielenie;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}

	void reszta()
	{
		std::cout << "Operacja: reszta z dzielenia" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::reszta_z_dzielenia;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
	void potega()
	{
		std::cout << "Operacja: potega" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::potega;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
	void suma()
	{
		std::cout << "Operacja: suma bitowa" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::suma_bitowa;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
	void XOR()
	{
		std::cout << "Operacja: bitowa alternatywa wykluczająca" << std::endl;
		TS8P t = wczytaj();
		t.operacja << TS8P::Operacja::suma_bitowa;
		t.status << TS8P::Status::wykonaj;
		send(t);
	}
};

int main()
{
	system("color 0a");
	setlocale(LC_ALL, "pl_PL");
	unsigned int wybor;
	do {
		std::cout << "Wybierz program:\n 1- Server\n 2- Client\n 0- Exit\n>" << std::flush;
		std::cin >> wybor;
	} while (wybor > 2);

	if (wybor == 0) return 0;
	if (wybor == 1)
	{
		server s;
		s.run();
	}
	else if (wybor == 2)
	{
		client c;
		c.run();
	}
	return 0;
}