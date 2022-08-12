#include <nvml.h>    // nvml*
#include <stdio.h>   // popen, (s)printf
#include <string.h>  // str*
#include <unistd.h>  // getopt

#define PID_COUNT_MAX 10000
#define MAX_LEN 1000

/**
 * @brief nvidia-smi variant
 *
 * @param -c // to colorize texts
 * @param -d // get docker name for each pid, if possible
 */
int main(int argc, char* argv[]) {
    // options
    int args[] = {0, 0};  // color, docker
    char option;
    while ((option = getopt(argc, argv, ":if:cd")) != -1) {
        switch (option) {
            case 'c':
                args[0] = 1;
                break;
            case 'd':
                args[1] = 1;
                break;
            case '?':
                // printf("unknown: %c\n", optopt);
                break;
        }
    }

    // params to colorize texts
    float thresholds[] = {0.9, 0.75, 0.2, 0};
    char fgcolors[][7] = {
        "\033[34m",  // purple
        "\033[31m",  // red
        "\033[33m",  // yellow
        "\033[32m",  // green
    };               // ANSI color
    char normal[] = "\033[0m";

    // first initialize NVML library
    nvmlReturn_t state = nvmlInit();
    if (NVML_SUCCESS != state) {
        printf("Failed: %s\n", nvmlErrorString(state));
        return 1;
    }

    // get number of GPUs
    unsigned int device_count;
    state = nvmlDeviceGetCount(&device_count);
    if (NVML_SUCCESS != state) {
        printf("Failed: %s\n", nvmlErrorString(state));
        return 1;
    }

    /*
    UPPER PART
    */
    char hline[] = "+------------------------------------------------------------------------------------------+\n";
    printf("%s", hline);
    printf("| %3s %27s %8s %10s %10s %25s |\n", "GPU", "NAME", "TEMP", "GPU-UTIL", "MEM-UTIL", "MEMORY-USAGE");
    printf("%s", hline);

    unsigned int pid_count = 0;                     // number of pids
    unsigned int all_pids[PID_COUNT_MAX];           // all of pids
    unsigned int gpu_indices[PID_COUNT_MAX];        // gpu index of each pid
    unsigned long long gpu_mems[PID_COUNT_MAX][2];  // GPU memory of each pid
    for (unsigned int i = 0; i < device_count; i++) {
        nvmlDevice_t device;
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        unsigned int temp;
        nvmlUtilization_t utilization;
        nvmlMemory_t memory;

        // get handler
        state = nvmlDeviceGetHandleByIndex(i, &device);
        if (NVML_SUCCESS != state) {
            printf("Failed, device %i: %s\n", i, nvmlErrorString(state));
            return 1;
        }

        // get device name
        state = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (NVML_SUCCESS != state) {
            printf("Failed, device %i: %s\n", i, nvmlErrorString(state));
            return 1;
        }

        // get device temperature
        state = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
        if (NVML_SUCCESS != state) {
            printf("Failed, device %i: %s\n", i, nvmlErrorString(state));
            return 1;
        }

        // get device utilization rates
        state = nvmlDeviceGetUtilizationRates(device, &utilization);
        if (NVML_SUCCESS != state) {
            printf("Failed, device %i: %s\n", i, nvmlErrorString(state));
            return 1;
        }

        // get memory information
        state = nvmlDeviceGetMemoryInfo(device, &memory);
        if (NVML_SUCCESS != state) {
            printf("Failed, device %i: %s\n", i, nvmlErrorString(state));
            return 1;
        }

        char usage[50];
        sprintf(usage, "%lluMiB / %lluMiB", memory.used >> 20, memory.total >> 20);
        // make color
        if (args[0]) {
            float ratio = 1.0f * memory.used / memory.total;
            for (int t = 0; t < 4; t++) {
                if (ratio >= thresholds[t]) {
                    printf("|%s %3d %27s %7dC %9u%% %9u%% %25s %s|\n",
                           fgcolors[t], i, name, temp, utilization.gpu, utilization.memory, usage, normal);
                    break;
                }
            }
        } else
            printf("| %3d %27s %7dC %9u%% %9u%% %25s |\n", i, name, temp, utilization.gpu, utilization.memory, usage);

        // get all pids using GPUs
        unsigned int infoCount = 1000;  // max pid is 1k per GPU
        nvmlProcessInfo_t infos[1000];
        state = nvmlDeviceGetComputeRunningProcesses(device, &infoCount, infos);
        if (NVML_SUCCESS != state) {
            printf("Failed, device %i: %s\n", i, nvmlErrorString(state));
            return 1;
        }

        // save metadata for later use
        for (unsigned int k = 0; k < infoCount; k++) {
            gpu_mems[pid_count + k][0] = infos[k].usedGpuMemory >> 20;
            gpu_mems[pid_count + k][1] = memory.total >> 20;
            all_pids[pid_count + k] = infos[k].pid;
            // printf("%u\n", infos[k].pid);
            gpu_indices[pid_count + k] = i;
        }
        pid_count += infoCount;
    }
    printf("%s", hline);

    /*
    LOWER PART
    */
    printf("%s", hline);
    printf("| %3s %8s %15.15s %9s %5s %5s %11s %25.25s |\n", "GPU", "PID", "USER", "GPU-MEM", "%CPU", "%MEM", "ELAPSED", "COMMAND");
    printf("%s", hline);

    // docker
    char cids[100000] = "";   // concatenate all container ids
    int usrs[PID_COUNT_MAX];  // 1: user is docker.name, 0: user is outside of docker
    char cgroup[1000];        // content of /proc/pid/cgrop
    // TODO: why there are non-exist pids???
    for (unsigned int i = 0; i < pid_count && args[1]; i++) {
        char path[128] = "/proc";
        sprintf(path, "%s/%u/cgroup", path, all_pids[i]);
        // puts(path);
        FILE* ffp = fopen(path, "r");
        if (ffp) {  // check file existence
            if (fgets(cgroup, 1000, ffp) != NULL)
                ;
            char* s = strstr(cgroup, "docker-");
            if (s) {
                char cid[13] = "";
                strncpy(cid, s + 7, 12);
                sprintf(cids, "%s %s", cids, cid);
                usrs[i] = 1;
            } else
                usrs[i] = 0;
            fclose(ffp);
        }
    }
    char out1[100];  // store docker.container.name for each pid
    FILE* fpp1 = NULL;
    if (args[1]) {
        char cmd[10000] = "docker inspect --format {{.Name}} ";
        strcat(cmd, cids);
        fpp1 = popen(cmd, "r");
    }

    char pids[100000] = "";  // concatenate all pids into a string
    if (pid_count > 0)       // there's at least a process
        sprintf(pids, "%s%u", pids, all_pids[0]);
    for (unsigned int i = 1; i < pid_count; i++)  // The rest processes
        sprintf(pids, "%s,%u", pids, all_pids[i]);
    char cmd[100000] = "ps -o user,%cpu,%mem,etime,command -q";
    sprintf(cmd, "%s %s", cmd, pids);

    // get some info given a pid
    char out2[MAX_LEN];  // content of each line
    FILE* fpp2 = popen(cmd, "r");
    if (fgets(out2, MAX_LEN, fpp2) != NULL)
        ;  // ignore header
    // get body
    for (unsigned int i = 0; i < pid_count && fgets(out2, MAX_LEN, fpp2) != NULL; i++) {
        // user, cpu, mem, etime (ELAPSED), command
        char pinfo[5][1000];
        out2[strcspn(out2, "\n")] = 0;  // remove last `\n`

        // separate a string into substrings by space
        strcpy(pinfo[0], strtok(out2, " "));
        for (unsigned int i = 1; i < 4; i++)
            strcpy(pinfo[i], strtok(NULL, " "));
        strcpy(pinfo[4], strtok(NULL, ""));  // leave the rest to this

        // get docker.container.name if usrs[i] is true
        if (args[1] && usrs[i]) {
            if (fgets(out1, 100, fpp1) != NULL) {
                memmove(out1, out1 + 1, strlen(out1));  // remove first `/`
                out1[strcspn(out1, "\n")] = 0;          // remove last `\n`
                strcpy(pinfo[0], out1);
            }
        }

        // make color
        if (args[0]) {
            float ratio = 1.0f * gpu_mems[i][0] / gpu_mems[i][1];
            for (int t = 0; t < 4; t++) {
                if (ratio >= thresholds[t]) {
                    printf("|%s %3u %8u %15.15s %6lluMiB %5s %5s %11s %25.25s %s|\n",
                           fgcolors[t], gpu_indices[i], all_pids[i], pinfo[0], gpu_mems[i][0], pinfo[1], pinfo[2], pinfo[3], pinfo[4], normal);
                    break;
                }
            }
        } else
            printf("| %3u %8u %15.15s %6lluMiB %5s %5s %11s %25.25s |\n",
                   gpu_indices[i], all_pids[i], pinfo[0], gpu_mems[i][0], pinfo[1], pinfo[2], pinfo[3], pinfo[4]);
    }
    printf("%s", hline);
    if (fpp1 && args[1])
        pclose(fpp1);
    if (fpp2)
        pclose(fpp2);
    return 0;
}