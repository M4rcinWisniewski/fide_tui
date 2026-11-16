#include <curl/curl.h>
#include <limits.h>
#include <minizip/unzip.h>
#include <stdio.h>

void unzip_data(const char *zip_file_path, const char *output_dir) {
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
}

void get_data() {
  CURL *curl;
  FILE *fp;
  CURLcode res;
  const char *url = "https://ratings.fide.com/download/players_list.zip";
  const char *zip_path = "/home/muslie/dev/fide_tui/ratings.zip";

  curl = curl_easy_init();
  if (curl) {
    fp = fopen(zip_path, "wb");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
  }

  unzip_data(zip_path, "/home/muslie/dev/fide_tui");
  if (remove(zip_path) != 0) {
    perror("Could not delete ZIP");
  }
}
