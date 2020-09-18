# include <cstdlib>
# include <iostream>
# include <omp.h>

//int main (int argc , char * argv [])
int main ()
{
//# pragma omp parallel
    {
        const int tid = omp_get_thread_num ();
        const int nthreads = omp_get_num_threads ();
        const int num_proc = omp_get_num_procs ();
        std :: cout<<"hello word" <<std::endl;
        std :: cout << " Hello   from   thread  " << tid
                    << " / " << nthreads
                    << " : " << num_proc << std :: endl ;
        return 0;
    }
}