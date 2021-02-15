#include "../inc/Destruction.h"
#include "../inc/Exec.h"
#include "../inc/other.h"


using namespace std;


Destruction::Destruction(wstring path_prog)
{
    a_path_prog = path_prog;
}


void Destruction::createBatchFile()
{
    HANDLE hFile; //use in CreateFile and WriteFile
    wstring content = L"@echo off\ntimeout 5\ndel " + a_path_prog + L"\n"; // use in WriteFile
    wstring command_attrib = L"attrib +h " + a_name_file_batch; ////command for hide the file batch

    DWORD dwBytesToWrite = (DWORD) content.length();// use in WriteFile
    DWORD dwBytesWritten = 0; // use in WriteFile
    BOOL status; // use in WriteFile

    hFile = CreateFileW(a_name_file_batch.c_str(),  GENERIC_WRITE,  0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL); //Crée ou ouvre un fichier ou un périphérique d'E / S. 
   
    if (GetLastError() == ERROR_FILE_EXISTS)
    {
        delete_batch_file();
        hFile = CreateFileW(a_name_file_batch.c_str(),  GENERIC_WRITE,  0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL); //Crée ou ouvre un fichier ou un périphérique d'E / S. 
    }
    
    if(hFile == INVALID_HANDLE_VALUE)
    {
        //error when creating the batch file.
        //cout << "impossible to open or write to the file. "<< GetLastError() << endl;
        a_error = TRUE;
    }//ERROR_ALREADY_EXISTS


    status = WriteFile(          //https://stackoverflow.com/questions/28618715/c-writefile-unicode-characters
                hFile,           // open file handle
                ConvertWideToUtf8(content).c_str(), // start of data to write
                dwBytesToWrite,  // number of bytes to write
                &dwBytesWritten, // number of bytes that were written
                NULL);           // no overlapped structure

    if(status == 0)
    {
        //If error
        a_error = TRUE;
    }
    CloseHandle(hFile);
    
    _wsystem(command_attrib.c_str()); //hide the file batch

}   


void Destruction::kills_all_same_process() 
{
    vector <DWORD> pids;
    wstring current_prog  = find_name_process(); //To monitor !!!!
    wcout << "\n\nName prog: "<<current_prog << endl;
    DWORD my_pid = GetCurrentProcessId();
    
    pids = Exec().returnPid(current_prog);

    if(!pids.empty())
    {
        for(int i=0;i<pids.size() ; i++)
        {
            HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pids[i]);
                
            TerminateProcess(proc, 1);
            CloseHandle(proc);
        }
    }
    else
    {
        //Empty pid
        ;
    }
}


void Destruction::startBatchFile()
{
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFOW siStartInfo;
    BOOL bSuccess = FALSE; 

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION)); 
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    siStartInfo.cb = sizeof(STARTUPINFO); 


    wstring argument_string = L"/C " + a_name_file_batch + L" && del " + a_name_file_batch + L"/A";
    
    WCHAR argument[argument_string.length()+1];

    wcscpy(argument, argument_string.c_str());

    bSuccess = CreateProcessW(L"C:\\windows\\system32\\cmd.exe",  //command line 
        argument,     // argv of cmd
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        FALSE,          // handles are inherited 
        CREATE_NO_WINDOW,             // creation flags 
        FALSE,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION
}


int Destruction::main()
{
    //-----------------------
    createBatchFile();
    if(testIfError())
    {
        return 1;
    }
    //-----------------------
    kills_all_same_process();
    if(testIfError())
    {
        return 1;
    }
    //system("pause");

    //-----------------------
    startBatchFile();
    if(testIfError())
    {
        return 1;
    }

    //-----------------------

    if(testIfError()) //test final
    {
        return 1;
    }
    else
    {return 0;}
}


int Destruction::testIfError()
{
    if(a_error)
    {
        delete_batch_file();
        return 1;
    }
    else
    {return 0;}
}


void Destruction::delete_batch_file()
{
    //Test if file batch exist:
    DWORD status = GetFileAttributesW(a_name_file_batch.c_str());
    
    if(status == INVALID_FILE_ATTRIBUTES)
    {
        //THE FILE DOESN'T EXIST ANYMORE
        //cout << "LE FICHIER N'EXISTE PLUS. " << endl; 
        ;
    }
    else
    {
        //THE FILE EXISTS 
        a_error = TRUE;
        //error (file not exist or file with higher rights)
        //system("pause");
        DeleteFileW(a_name_file_batch.c_str());
    }

}//https://stackoverflow.com/questions/4403986/c-which-is-the-best-method-of-checking-for-file-existence-on-windows-platform

Destruction::~Destruction()
{
    ;
}

wstring Destruction::find_name_process()
{
    int index = 0;
    for(int i=0; i < a_path_prog.length(); i++)
    {
        if(a_path_prog.at(i) == L'\\')
        {
            wcout << "back slash find: "<< i << endl;
            index = i;
        }
    }
    index += 1;

    return a_path_prog.substr(index);
}
