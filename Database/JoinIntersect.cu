// Author : Pinaggle
// Last Modified: 20018/11/2 14:38
#include <vector>
#include <iostream>
#include <iomanip>
#include <time.h>
using namespace std;

/*
 * this function is used to do multi-join on GPU, the lists to be joined is recorded
 * one by one in arr[],and the start position of each list is recorded in head[].
 * table saves which lists are to be intersected. It's size is join_length*item_size.
 * Each column of table represent a multi-way join, so we hacve item_size results at
 * last.
 */
void disp(const char* name,unsigned *arr,unsigned length,unsigned line)
{
 	cout << name << endl;
	for(unsigned i = 0;i < length;i++)
	{
		cout <<arr[i] << ' ';
		if(i % line == line-1)
			cout << endl;
	}
	cout << endl;
}
void start_clock(struct timespec &start)
{
	clock_gettime(CLOCK_MONOTONIC,&start);
}

void gettime_and_reset_clock(struct timespec &start, const char* info)
{
	struct timespec cnt;
	clock_gettime(CLOCK_MONOTONIC,&cnt);
	double period = 1000*(cnt.tv_sec - start.tv_sec)
			+ 0.000001*(cnt.tv_nsec - start.tv_nsec);
	cout << info << " used " << setiosflags(ios::fixed) 
		<< setprecision(2) << period <<"ms"<<endl;
	start = cnt;
}
void gettime(struct timespec &start, const char* info)
{
	
	struct timespec cnt;
	clock_gettime(CLOCK_MONOTONIC,&cnt);
	double period = 1000*(cnt.tv_sec - start.tv_sec)
			+ 0.000001*(cnt.tv_nsec - start.tv_nsec);
	cout << info << " used " << setiosflags(ios::fixed)
		<< setprecision(2) << period <<"ms"<<endl;
} 
void cudaInit()
{
	unsigned *d_init;
	cout << "CudaInitFinished" << endl;
	cudaMalloc(&d_init,sizeof(unsigned));
	cudaFree(d_init);
}
__global__ void binarySearch(unsigned *d_head,unsigned *d_arr,unsigned join_width,
		unsigned table_length,unsigned *d_can_list,unsigned can_list_size)
{
	unsigned idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned gridSize = gridDim.x*blockDim.x;

	while(idx < d_head[table_length])
	{
		unsigned key = d_arr[idx];
		unsigned l = 0,r = table_length,m,i;
		while(l+1 < r)
		{
			m = (l+r)/2;
			if(d_head[m] <= idx)
				l = m;
			else
				r = m;
		}
		i = l;
		bool found = true;
		for(unsigned j = 1;j < join_width;j++)
		{
			l = d_head[j*table_length+i];
			r = d_head[j*table_length+i+1];
			while(l+1 < r)
			{
				m = (l+r)/2;
				if(d_arr[m] <= key)
					l = m;
				else 
					r = m;
			}
			if((l == r)||(d_arr[l] != key))
			{
				found = false;
				break;
			}
		}
		if(found && (d_can_list != NULL))
		{
			l = 0;
			r = can_list_size;
			while(l+1 < r)
			{
				m = (l+r)/2;
				if(d_can_list[m] <= key)
					l = m;
				else 
					r = m;
			}
			if((l == r)||(d_can_list[l] != key))
				found = false;
		}
		if(!found)
			d_arr[idx] = -1;
		idx += gridSize;
	}	

}
void calc(unsigned *head,unsigned *arr, unsigned join_width,unsigned table_length,
		unsigned item_num,unsigned *can_list,unsigned can_list_size)
{
#ifdef DEBUG_JOIN
	disp("head", &head[0], head.size(), table_length);
#endif
	// bool use_gpu = false;
	bool use_gpu = true;
	if(use_gpu)
	{
		cout << "enter GPU mode"<<endl;
		unsigned *d_head, *d_arr, *d_can_list = NULL;
		cudaMalloc(&d_head, sizeof(unsigned)*(join_width*table_length+1));
		cudaMalloc(&d_arr, sizeof(unsigned)*item_num);
		if(can_list != NULL)
			cudaMalloc(&d_can_list, sizeof(unsigned)*can_list_size);
		if (d_head == NULL || d_arr == NULL || (can_list != NULL && d_can_list == NULL))
		{
			cout << "GPU malloc failed!" << endl;
			if (d_head) cudaFree(d_head);
			if (d_arr) cudaFree(d_arr);
			if (d_can_list) cudaFree(d_can_list);
			goto CPUMODE;
		}
		cudaMemcpy(d_head, head, sizeof(unsigned)*(join_width*table_length+1), cudaMemcpyHostToDevice);
		cudaMemcpy(d_arr, arr, sizeof(unsigned)*item_num, cudaMemcpyHostToDevice);
		if(can_list != NULL)
			cudaMemcpy(d_can_list, can_list, sizeof(unsigned)*can_list_size, cudaMemcpyHostToDevice);
		int threadsPerBlock = 256;
		int blocksPerGrid = (head[table_length] + threadsPerBlock - 1) / threadsPerBlock;
		if(blocksPerGrid > 1024)
			blocksPerGrid = 1024;
		cudaEvent_t start,stop;
		float period;
		cudaEventCreate(&start);
		cudaEventCreate(&stop);
		cudaEventRecord(start,0);
		binarySearch <<<blocksPerGrid, threadsPerBlock >>> (d_head,d_arr,
				join_width,table_length,d_can_list,can_list_size);
		cudaEventRecord(stop,0);
		cudaEventSynchronize(stop);
		cudaEventElapsedTime(&period,start,stop);
		printf("****Kernel Execution time: %.2f ms\n",period);
		cudaMemcpy(arr, d_arr, sizeof(unsigned)*head[table_length], cudaMemcpyDeviceToHost);
		cudaFree(d_head);
		cudaFree(d_arr);
		if(can_list != NULL)
			cudaFree(d_can_list);
	}
	else
	{
CPUMODE:
		cout << "enter CPU mode" << endl;
		struct timespec start;
		for (unsigned i = 0; i < table_length; i++)
			for(unsigned idx = head[i];idx < head[i+1];idx++) 
		{
			unsigned key = arr[idx];
			unsigned l,r,m;
			bool found = true;
			for (unsigned j = 1; j < join_width; j++)
			{
				l = head[j*table_length+i];
				r = head[j*table_length+i+1];
				while (l + 1 < r)
				{
					m = (l + r) / 2;
					if (arr[m] <= key)
						l = m;
					else
						r = m;
				}
				if ((l == r) || (arr[l] != key))
				{
					found = false;
					break;
				}
			}
			if(found && can_list != NULL)
			{
				l = 0;
				r = can_list_size;
				while (l + 1 < r)
				{
					m = (l + r) / 2;
					if (can_list[m] <= key)
						l = m;
					else
						r = m;
				}
				if ((l == r) || (can_list[l] != key))
				{
					found = false;
				}
			}
			if(!found)
				arr[idx] = -1;			
		}
	}
}
