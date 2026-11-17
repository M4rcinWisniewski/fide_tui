#include <curl/curl.h>
#include <errno.h>
#include <limits.h>
#include <minizip/unzip.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
void unzip_data(const char *zip_file_path, const char *output_dir) {
  if (mkdir(output_dir, 0755) != 0 && errno != EEXIST) {
    perror("mkdir");
    return;
  }

  unzFile zip_file = unzOpen(zip_file_path);
  if (zip_file == NULL) {
    fprintf(stderr, "Cannot open ZIP: %s\n", zip_file_path);
    return;
  }

  if (unzGoToFirstFile(zip_file) == UNZ_OK) {
    do {
      unz_file_info file_info;
      char file_name[256];

      if (unzGetCurrentFileInfo(zip_file, &file_info, file_name,
                                sizeof(file_name), NULL, 0, NULL,
                                0) != UNZ_OK) {
        fprintf(stderr, "Cannot read ZIP entry name\n");
        break;
      }

      if (unzOpenCurrentFile(zip_file) != UNZ_OK) {
        fprintf(stderr, "Cannot open ZIP entry: %s\n", file_name);
        continue;
      }

      char output_path[PATH_MAX];
      snprintf(output_path, sizeof(output_path), "%s/%s", output_dir,
               file_name);

      FILE *out = fopen(output_path, "wb");
      if (out == NULL) {
        perror(output_path);
        unzCloseCurrentFile(zip_file);
        continue;
      }

      char buffer[8192];
      int bytes_read;
      while ((bytes_read =
                  unzReadCurrentFile(zip_file, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytes_read, out) != (size_t)bytes_read) {
          perror("fwrite");
          break;
        }
      }

      fclose(out);
      unzCloseCurrentFile(zip_file);
    } while (unzGoToNextFile(zip_file) == UNZ_OK);
  }

  unzClose(zip_file);

  // Rename inside the output directory
  char old_path[PATH_MAX], new_path[PATH_MAX];
  snprintf(old_path, sizeof(old_path), "%s/players_list_foa.txt", output_dir);
  snprintf(new_path, sizeof(new_path), "%s/ratings.txt", output_dir);
  rename(old_path, new_path);
}

bool does_file_exist(char *file_path) {
  FILE *check_file = fopen(file_path, "r");
  if (check_file == NULL) {
    return false;
  }
  fclose(check_file);
  return true;
}

void get_data() {
  if (!does_file_exist("./data/ratings.txt")) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    const char *url = "https://ratings.fide.com/download/players_list.zip";
    const char *zip_path = "/home/muslie/dev/fide_tui/ratings.zip";
    const char *data_dir = "/home/muslie/dev/fide_tui/data";

    curl = curl_easy_init();
    if (curl) {
      fp = fopen(zip_path, "wb");
      curl_easy_setopt(curl, CURLOPT_URL, url);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      fclose(fp);
    }

    unzip_data(zip_path, data_dir);
    if (remove(zip_path) != 0) {
      perror("Could not delete ZIP");
    }
  } else {
    puts("File already downloaded!");
  }
}
