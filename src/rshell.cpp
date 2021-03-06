#include<iostream>
#include <sys/stat.h>
#include <vector>
#include<unistd.h> //for execvp
#include<stdio.h> //for perror
#include<errno.h> //perror 
#include<sys/types.h> //for wait
#include<sys/wait.h> //for wait
#include<string>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

using namespace std;

int new_proc(int in, int out, char ** cmd)
{
    //cout << "in new proc " << endl;
    pid_t pid;

    if( (pid = fork()) == -1 )
    {
            perror("fork:21");
            exit(1);
    }
    else if(pid == 0) //child
    {
        if(in != 0) //
        {
            if(-1 == dup2(in, 0))
            {
                perror("dup2 32");
                exit(1);
            }
            close(in); //close it. Copied to STDOUT
        }
        if(out != 1) //
        {
            if(-1 == dup2(out, 1))
             {
                perror("dup2 41");
                exit(1);       
             }//we will write to pipe
            close(out);//close it. Copied it to STDIN
        }
        if( -1 == execvp(cmd[0], cmd))
        {
                perror("execvp:37");
                exit(1);
        }
    }
    return pid; //IDK?
}

int fork_pipe(int n, vector<string> arg_list, vector<int> redir_flags)
{
    //cout << "HERE " << endl;
    int in, fd[2];
    in = 0; //first process should read from STDIN... unless other redir flag set
            //determine redir flag below! ! ! ! ! ! ! -------------------------------->  ! ! ! ! ! ! !  ! !
    char *token_temp, *savptr_temp, *replace_temp;
    char del2[] = "< >";
    vector<string> single_cmd;
    replace_temp = new char[arg_list.at(0).size()+1];

    char **argv = new char*[arg_list.at(0).size()+1];
    strcpy(replace_temp, arg_list.at(0).c_str());
    token_temp = strtok_r(replace_temp, del2, &savptr_temp);

//-----------------------------------------------------------------------------
    //cout << "68: i " <<  n << endl;    
    for(int i=0; i < n-1; i++) //all but last processes created here
    {
        //cout << "loop " <<  " i " << i << endl;
        strcpy(replace_temp, arg_list.at(i).c_str());    
        token_temp = strtok_r(replace_temp, del2, &savptr_temp);

        while(token_temp != 0)
        {
            single_cmd.push_back(token_temp);
            //cout << "74 " << token_temp << endl;
            token_temp = strtok_r(NULL, del2, &savptr_temp);
        }
        for(unsigned j=0; j < single_cmd.size(); j++)
        {
            argv[j] = new char[single_cmd.at(j).size()+1];
            strcpy(argv[j], single_cmd.at(j).c_str());
            //cout << "81 " << argv[j] << endl;
            argv[single_cmd.size()] = 0;
        }
        single_cmd.clear();//clear up vector

        //cout << "88" << argv[0] << endl;
        if(-1 == pipe(fd))
        {
            perror("pipe 99");
            exit(1);
        }
        //fd[1]write end of the pipe we carry in from prev iteration....
        
        //need to change STDIN if input redir
        //cout << " i " << i << endl;
        //cout << redir_flags.at(i) << " 95 " <<  endl;
        if(redir_flags.at(i) == 0)
        {
            if( -1 == (in = open(argv[1], O_RDONLY | O_CREAT, 00700)))
            {
                perror("open:94");
                exit(1);
            }
            //cout << in  << " 100 "<< endl;
            
        }
        else if(redir_flags.at(i) == 1)
        {
            if(-1 == (in = open(argv[0], O_RDWR | O_CREAT, 0777)))
            {
                perror("open:105 ");
                exit(1);
            }
            if(-1 == dup2(in,1))
            {
                perror("dup2 124");
                exit(1);
            }
            close(in);
        }
        else if(redir_flags.at(i) == 2)
        {
            //cout << "out2 115 " << endl;
            if(-1 == (in = open(argv[0], O_RDWR | O_APPEND, 0777)))
            {
                perror("open:117 ");
                exit(1);
            }
        }
        //cout << "in 123 " << in << endl;
        new_proc(in, fd[1], argv);

        //dont need to write to pipe, child does that
        close(fd[1]);
        //cout << "returned 128" << endl;
        //cout << "i " << i << endl;
        //keep read end of pipe, next child will read from here
        in = fd[0];

     }
//-----------------------------------------------------------------------------
        //cout << "134 " <<  token_temp << endl;
        
        if(n != 0)
        {
            strcpy(replace_temp, arg_list.at(n-1).c_str());    
            token_temp = strtok_r(replace_temp, del2, &savptr_temp);
        }
        while(token_temp != 0)
        {
            single_cmd.push_back(token_temp);
            //cerr << "142 " << token_temp << endl;
            token_temp = strtok_r(NULL, del2, &savptr_temp);
        }
        int size_curr_parse = single_cmd.size();
        //cerr << "144 " << endl;
        
        argv = new char*[single_cmd.size()+1]; 
        for(unsigned j=0; j < single_cmd.size(); j++)
        {
            argv[j] = new char[single_cmd.at(j).size()+1];
            strcpy(argv[j], single_cmd.at(j).c_str());
            //cout << "149 " << argv[j] << endl;
            //cout << "command size 149 " << single_cmd.size() << endl;
            //cout << "j " << j << endl;
            //argv[arg_list.size()+1] = 0;
        }
        for(int k=0; k < size_curr_parse; k++)
        {
            //cout << "166: " << argv[k] << endl;
        }
        //cout << redir_flags.size() << " flag size " << endl;
        
        
     //last stage of pipe, set STDIN to be read end of prev pipe and output 
        //to the original fd 1
        if(in != 0)
        {
            if(-1 == dup2(in, 0))
            {
                perror("dup2 189");
                exit(1);
            }
        }

        //case where only one cmd "cat < test.cpp"
        if(redir_flags.at(n-1) == 1)
        {
            //cout << "good, made it " << endl;
            //cout << argv[size_curr_parse-1] << endl;
            if(-1 == (in = open(argv[size_curr_parse-1], O_RDWR| O_TRUNC | O_CREAT, 00700)))
            {
                perror("open 182");
                exit(1);
            }
            
            argv[size_curr_parse-1] = 0;
            if(-1 == dup2(in, 1))
            {
                perror("dup2 208");
                exit(1);
            }
            close(in);
        }
        if(redir_flags.at(n-1) == 2)
        {
            //cout << "good, made it2 " << endl;
            //cout << argv[size_curr_parse-1] << endl;
            if(-1 == (in = open(argv[size_curr_parse-1], O_RDWR | O_APPEND, 00700)))
            {
                perror("open 182");
                exit(1);
            }
            
            argv[size_curr_parse-1] = 0;
            if(-1 == dup2(in, 1))
            {
                perror("dup2 226");
                exit(1);
            }
            close(in);
        }
        if(redir_flags.at(0) == 0 && redir_flags.size() == 1)
        {
            //cout << "input redir 161 " << endl;
            if(n==1)
            {
                //cout << "168 " << endl;
                //cout << "173 " << argv[1] << endl;
                if(-1 == (in = open(argv[1], O_RDONLY | O_CREAT, 00700)))
                {
                    perror("open 167");
                    exit(1);
                
                }
                close(0);
                if(-1 == dup2(in,0))
                {
                    perror("dup2 247");
                    exit(1);
                }
            }
            else if(-1 == (in = open(argv[0], O_RDONLY | O_CREAT, 00700)))
            {
                perror("open 254");
                exit(1);
                close(0);
                if(-1 == dup2(in,0))
                {
                    perror("dupe 168 ");
                    exit(1);
                }
            }
            close(0);
            /*if(-1 == dup2(in,0))
            {       
                perror("dup2 263");
                exit(1);
            }*/
        }
        /*else if(redir_flags.at(i) == 1 && redir_flags.size() == 1)
        {
            cout << "out 170 " << endl;
            cout << size_curr_parse << " size pasre" << endl;
            cout << "203 " << argv[size_curr_parse-1] << endl;
            if(-1 == (in = open(argv[size_curr_parse-1], O_RDWR | O_CREAT, 00700)))
            {
                perror("open:175 ");
                exit(1);
            }
            dup2(in,1);
            close(in);
            //argv[arg_list.size()-1] = 0;
        }*/
        /*else if(redir_flags.at(i) == 2 && redir_flags.size() ==1)
        {
            cout << "out2 178 " << endl;
            if(-1 == (in = open(argv[size_curr_parse-1], O_RDWR | O_APPEND, 00700)))
            {
                perror("open:252 ");
                exit(1);
            }
            dup2(in,1);
            close(in);
        }*/
        /*if(in != 0)
        //input and output to original fd
        {
            cout << "298 " << endl;
            if(-1 == dup2(in,0))
            {
                perror("dup2 298");
                exit(1);
            }
        }*/

        //execute last stage with current proc
        //cout << "execute betch " << endl;
        if(-1 == execvp(argv[0], argv))
        {
            perror("execvp 222");
            exit(1);
        }
        return 1;
}

int main ()
{
        while (1)
        {   //infinite loop to run until `exit` command
            char* login = getlogin();
            //gets user info/id && prints to terminal
            if ( 0 ==  login)
            {
                perror("getLogin()");
            }
            char user[256];
            if (-1 == gethostname(user, sizeof(user)))
            {
                perror("gethostname");
            }
            int findComment;
            string parse="";
            //line outputs user info before prompt
            cout << login << user << "$ ";
            getline(cin, parse);
            findComment = parse.find("#");
            //all these variable will allow me to see what is contained in user input
            int findAnd, findPipe;//, findInRedir, findOutRedir;
            findAnd = parse.find("&&");
            findPipe = parse.find("|");
            int orTrip=0,andTrip=0;
            //if find #, replace with null so that parsing treats it as end of array
            //source of one of my bugs
            if(findComment != -1)
            {
                parse.at(findComment) = 0;
                //cout << "found comment: " << endl;
            }
            if(findAnd != -1) //signal if user input has && 
            {
                andTrip = 1;
                //cout << "found `&&`: " << endl;

            }
            if(findPipe !=-1 )//need to check if pipe or `||`
            {
                if(parse[findPipe + 1] == '|')
                {
                    //cout << "found ||" << endl;
                    orTrip = 1;
                }
                
                               
            }
            char del[] = ";|&"; //delimiter to signal diff cmd
            char *token, *token2;
            char *savptr1, *savptr2;
                           
            char* replace=0; 
            replace = new char[parse.size()+2];
            strcpy(replace, parse.c_str());

            token = strtok_r(replace, del, &savptr1);
//--------------------------OLD RSHELL ----------------------------------------
            //cout << "ortrip " << orTrip << endl;
            //cout << andTrip << " andTrip " << endl;
            string quit = token;
            if(quit  == "exit")
            {
                exit(1);
            }   
            if(orTrip !=0 || andTrip !=0)
            {
                    //cout << "here " << endl;
                    while (token != NULL)
                    {
                            //cout << "pased token first round: " << endl;
                            //cout << token << endl;
                            string fakeTok = token;
                            char* replace2=0;
                            replace2 = new char[fakeTok.size()+1];

                            char delim2[] = " "; //to parse further.. take out spaces
                            strcpy(replace2, fakeTok.c_str());

                            token2 = strtok_r(replace2, delim2, &savptr2);
                            vector<string> hold;
                            //put parsed data into vector for ease of access
                            while(token2 != NULL)
                            {
                                    hold.push_back(token2);
                                    token2 = strtok_r(NULL, delim2, &savptr2);
                            }

                            //loop to get parsed data into array for execvp
                            char **argv = new char*[hold.size()+1];
                            for(unsigned i=0; i<hold.size();i++)
                            {
                                    if(hold.at(i) == "exit")
                                    {
                                            exit(1);
                                    }
                                    argv[i] = new char[hold.at(i).size()+1];
                                    strcpy(argv[i], hold.at(i).c_str());
                                    argv[hold.size()] = 0;
                            }

                            //fork to actually execute cmds 
                            int pid2 = fork();
                            int status = 0;
                            if(pid2 == -1)
                            {
                                    perror("fork");
                                    exit (1);
                            }
                            else if (pid2 == 0) //child
                            {
                                    if( -1 == execvp(argv[0], argv))
                                    {
                                            perror("execvp");
                                            if(andTrip == 1)
                                            {
                                                    exit(1);
                                            }
                                            else if(orTrip == 1)
                                            {
                                                    exit(1);
                                            }
                                            exit(1);
                                    }
                                    exit(1);
                            }
                            else //parent
                            {   
                                    if(-1 == wait(&status))
                                    {
                                            perror("wait()");
                                    }
                                    else
                                    {  
                                            if(status > 0 && andTrip == 1) //break out if && failed
                                            {
                                                    break;

                                            }
                                            if(WEXITSTATUS(status)==0 && orTrip==1) //break if cmd succeeds and || cmd 
                                            {
                                                    break;
                                            }
                                            else
                                            {
                                                    //inc token
                                                    token = strtok_r(NULL, del, &savptr1);
                                            }
                                    }
                            }
                    }
            }
//---------------------REVISED RSHELL -----------------------------------------
            else
            {
                //cerr << "down here " << endl; 
                string token4; //
                char* replace2=0; //
                replace2 = new char[parse.size()+2]; //
                vector<string> tokens; //
                vector<int> redirect_indicator; //
                strcpy(replace2, replace); //
            
//-----------------PUT CMD STRINGS  INTO VECTOR FOR EASE-----------------------
                while(token != NULL)
                {
                    token4  = token;
                    //cout << "215 " <<  token << endl;

                    //will push indicators into vector for reference in pipe_fork
                    if((int)token4.find("<")!=-1){
                        redirect_indicator.push_back(0);
                        //cout << "pushed 0 " << endl;
                    }
                    else if((int)token4.find('>')!=-1)
                    {
                        if(token4[token4.find('>')+1] == '>')
                        {
                            redirect_indicator.push_back(2);
                            //cout << "pushed 2: " << endl;
                        }
                        else{
                            redirect_indicator.push_back(1);
                            //cout << "pushed 1 " << endl;
                        }
                    }
                    else{
                        redirect_indicator.push_back(-1);
                        //cout << "pushed -1 " << endl;
                    }

                    tokens.push_back(token);
                
                
                    token = strtok_r(NULL, del, &savptr1);
                }
                fork_pipe(tokens.size(), tokens, redirect_indicator);
                //cout << "returned fully " << endl;
            }
            
        }
        return 0;
}





















