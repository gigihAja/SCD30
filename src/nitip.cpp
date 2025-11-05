// // // #include "scd30_sensor.h"

// // // static Adafruit_SCD30 g_scd30;

// // // namespace Sensors
// // // {

// // //     bool SCD30_begin(uint16_t interval_sec, bool enable_auto_cal)
// // //     {
// // //         // scd30.begin() akan pakai Wire default (I2C sudah diinit oleh kode lain)
// // //         if (!g_scd30.begin())
// // //             return false;
// // //         g_scd30.setMeasurementInterval(interval_sec);
// // //         g_scd30.selfCalibrationEnabled(enable_auto_cal);
// // //         return true;
// // //     }

// // //     bool SCD30_read(float &co2_ppm, float &temp_c, float &rh_pct)
// // //     {
// // //         if (!g_scd30.dataReady())
// // //             return false;
// // //         if (!g_scd30.read())
// // //             return false;
// // //         co2_ppm = g_scd30.CO2;
// // //         temp_c = g_scd30.temperature;
// // //         rh_pct = g_scd30.relative_humidity;
// // //         return true;
// // //     }

// // // } // namespace Sensors

// // // #include <Wire.h>
// // // #include <Adafruit_SCD30.h>

// // // Adafruit_SCD30 scd30;

// // // void setup()
// // // {
// // //     Serial.begin(115200);
// // //     delay(1000);

// // //     // Paksa ESP32 pakai pin I2C default: SDA=21, SCL=22
// // //     Wire.begin(21, 22);

// // //     if (!scd30.begin())
// // //     {
// // //         Serial.println("SCD30 tidak terdeteksi. Cek wiring & pull-up.");
// // //         while (1)
// // //             delay(10);
// // //     }

// // //     // Interval baca (detik). Minimum 2 s.
// // //     if (!scd30.setMeasurementInterval(2))
// // //     {
// // //         Serial.println("Gagal set interval, lanjut default.");
// // //     }

// // //     // (Opsional) Matikan Auto Self-Calibration kalau alat dipakai di lab yang CO2-nya tidak sering 'kembali' ke 400 ppm
// // //     scd30.selfCalibrationEnabled(true); // atau false kalau mau dimatikan

// // //     // (Opsional) Kompensasi ketinggian (meter) atau tekanan (mbar)
// // //     // scd30.setAltitude(50);       // contoh 50 m dpl
// // //     // scd30.setAmbientPressure(1013); // mbar

// // //     Serial.println("SCD30 siap. Tunggu stabilisasi awal ~2 menit...");
// // // }

// // // void loop()
// // // {
// // //     // Data baru tersedia tiap interval
// // //     if (scd30.dataReady())
// // //     {
// // //         if (!scd30.read())
// // //         {
// // //             Serial.println("Gagal baca data.");
// // //             delay(200);
// // //             return;
// // //         }
// // //         Serial.print("CO2: ");
// // //         Serial.print(scd30.CO2);
// // //         Serial.print(" ppm, ");
// // //         Serial.print("T: ");
// // //         Serial.print(scd30.temperature, 2);
// // //         Serial.print(" C, ");
// // //         Serial.print("RH: ");
// // //         Serial.print(scd30.relative_humidity, 2);
// // //         Serial.println(" %");
// // //     }
// // //     delay(200);
// // // }

// // // nitip 2

// // // #include <Arduino.h>
// // // #include <Wire.h>
// // // #include <Adafruit_SCD30.h>
// // // #include <Preferences.h>

// // // ///////////////////////
// // // // PIN & CONSTANTS
// // // ///////////////////////
// // // #define I2C_SDA_PIN 21
// // // #define I2C_SCL_PIN 22
// // // #define CAL_BUTTON_PIN 18 // tombol kalibrasi, aktif LOW
// // // #define BUTTON_DEBOUNCE_MS 200

// // // // Interval pembacaan sensor (ms)
// // // #define SAMPLE_INTERVAL_MS 5000

// // // // Key di NVS
// // // #define NVS_NAMESPACE "calib"
// // // #define NVS_KEY_OFFSET "co2_off"
// // // #define NVS_KEY_REFPPM "ref_ppm"

// // // // Default nilai referensi untuk kalibrasi (dalam %Vol).
// // // // Misal: udara bebas ~0.04 %Vol CO2 -> 400 ppm.
// // // // Ganti sesuai protokol kamu.
// // // // Di bawah ini aku set 0.04 %Vol.
// // // #define DEFAULT_REF_PERCENT 0.04f

// // // ///////////////////////
// // // // GLOBAL STATE
// // // ///////////////////////
// // // Adafruit_SCD30 scd30;
// // // Preferences prefs;

// // // float co2_offset_ppm = 0.0f; // offset kalibrasi (ppm), disimpan di NVS
// // // float ref_target_ppm = 0.0f; // nilai referensi (ppm) yang alat pembanding katakan benar
// // //                              // ini bisa diubah via Serial (REF x.xx) dan juga disimpan

// // // unsigned long lastSampleMs = 0;
// // // unsigned long lastButtonCheckMs = 0;
// // // bool lastButtonState = HIGH; // kita pakai pull-up -> HIGH = idle
// // // bool buttonWasPressed = false;

// // // ///////////////////////
// // // // UTILITIES
// // // ///////////////////////

// // // float ppmToPercentVol(float ppm)
// // // {
// // //     // 1%Vol = 10,000 ppm
// // //     return ppm / 10000.0f;
// // // }

// // // float percentVolToPPM(float percentVol)
// // // {
// // //     return percentVol * 10000.0f;
// // // }

// // // void loadCalibrationFromNVS()
// // // {
// // //     prefs.begin(NVS_NAMESPACE, true); // read-only
// // //     co2_offset_ppm = prefs.getFloat(NVS_KEY_OFFSET, 0.0f);
// // //     ref_target_ppm = prefs.getFloat(NVS_KEY_REFPPM, percentVolToPPM(DEFAULT_REF_PERCENT));
// // //     prefs.end();
// // // }

// // // void saveCalibrationToNVS()
// // // {
// // //     prefs.begin(NVS_NAMESPACE, false); // write
// // //     prefs.putFloat(NVS_KEY_OFFSET, co2_offset_ppm);
// // //     prefs.putFloat(NVS_KEY_REFPPM, ref_target_ppm);
// // //     prefs.end();
// // // }

// // // // Hitung offset baru berdasar pembacaan sensor saat ini dan ref_target_ppm
// // // // offset = ref - measured
// // // // lalu simpan
// // // void performCalibration(float measured_ppm)
// // // {
// // //     co2_offset_ppm = ref_target_ppm - measured_ppm;
// // //     saveCalibrationToNVS();

// // //     Serial.println(F("=== KALIBRASI DISIMPAN ==="));
// // //     Serial.print(F("Measured (ppm): "));
// // //     Serial.println(measured_ppm, 2);
// // //     Serial.print(F("Reference (ppm): "));
// // //     Serial.println(ref_target_ppm, 2);
// // //     Serial.print(F("Offset baru (ppm): "));
// // //     Serial.println(co2_offset_ppm, 2);
// // //     Serial.println(F("=========================="));
// // // }

// // // // print status lengkap
// // // void printStatus(float latest_meas_ppm, bool haveLatest)
// // // {
// // //     Serial.println(F("\n----- STATUS -----"));

// // //     if (haveLatest)
// // //     {
// // //         float corrected_ppm = latest_meas_ppm + co2_offset_ppm;
// // //         Serial.print(F("CO2 raw (ppm): "));
// // //         Serial.println(latest_meas_ppm, 2);
// // //         Serial.print(F("CO2 raw (%Vol): "));
// // //         Serial.println(ppmToPercentVol(latest_meas_ppm), 4);

// // //         Serial.print(F("CO2 corrected (ppm): "));
// // //         Serial.println(corrected_ppm, 2);
// // //         Serial.print(F("CO2 corrected (%Vol): "));
// // //         Serial.println(ppmToPercentVol(corrected_ppm), 4);
// // //     }
// // //     else
// // //     {
// // //         Serial.println(F("Belum ada data sensor terbaru."));
// // //     }

// // //     Serial.print(F("ref_target_ppm (ppm): "));
// // //     Serial.println(ref_target_ppm, 2);
// // //     Serial.print(F("ref_target (%Vol): "));
// // //     Serial.println(ppmToPercentVol(ref_target_ppm), 4);

// // //     Serial.print(F("offset sekarang (ppm): "));
// // //     Serial.println(co2_offset_ppm, 2);
// // //     Serial.print(F("offset sekarang (%Vol): "));
// // //     Serial.println(ppmToPercentVol(co2_offset_ppm), 4);

// // //     Serial.println(F("Perintah Serial yang tersedia:"));
// // //     Serial.println(F("  REF <angka_percentVol>  -> set nilai referensi (contoh: REF 4.85)"));
// // //     Serial.println(F("  CAL                     -> hitung offset = REF - pembacaan saat ini, lalu simpan"));
// // //     Serial.println(F("  STATUS                  -> tampilkan status ini"));
// // //     Serial.println(F("  RESETCAL                -> hapus offset jadi 0 & simpan"));
// // //     Serial.println(F("--------------------\n"));
// // // }

// // // ///////////////////////
// // // // SETUP
// // // ///////////////////////
// // // void setup()
// // // {
// // //     Serial.begin(115200);
// // //     delay(1000);

// // //     // I2C init
// // //     Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

// // //     // Tombol kalibrasi
// // //     pinMode(CAL_BUTTON_PIN, INPUT_PULLUP);

// // //     // Load offset dan ref dari NVS
// // //     loadCalibrationFromNVS();

// // //     // Init sensor SCD30
// // //     if (!scd30.begin())
// // //     {
// // //         Serial.println(F("ERROR: SCD30 tidak terdeteksi. Cek wiring/power!"));
// // //         while (1)
// // //         {
// // //             delay(1000);
// // //         }
// // //     }

// // //     // Set interval pengukuran (detik). Min 2 detik.
// // //     if (!scd30.setMeasurementInterval(2))
// // //     {
// // //         Serial.println(F("Gagal set interval, pakai default internal sensor."));
// // //     }

// // //     // Kita matikan auto self calibration default dulu, supaya kita kontrol manual offset:
// // //     scd30.selfCalibrationEnabled(false);

// // //     // (Opsional) sesuaikan ke kondisi lab kamu:
// // //     // scd30.setAltitudeOffset(50);     // meter di atas permukaan laut
// // //     // scd30.setAmbientPressure(1013);  // mbar

// // //     Serial.println(F("SCD30 inisialisasi OK."));
// // //     Serial.println(F("System siap.\nKetik STATUS di Serial Monitor untuk lihat info.\n"));
// // // }

// // // ///////////////////////
// // // // LOOP
// // // ///////////////////////
// // // void loop()
// // // {
// // //     static float last_measured_ppm = NAN;
// // //     bool haveLatest = false;

// // //     // 1. Cek data sensor per SAMPLE_INTERVAL_MS
// // //     unsigned long now = millis();
// // //     if (now - lastSampleMs >= SAMPLE_INTERVAL_MS)
// // //     {
// // //         lastSampleMs = now;

// // //         if (scd30.dataReady())
// // //         {
// // //             if (scd30.read())
// // //             {
// // //                 float raw_ppm = scd30.CO2; // ppm dari library
// // //                 float corrected_ppm = raw_ppm + co2_offset_ppm;

// // //                 last_measured_ppm = raw_ppm;
// // //                 haveLatest = true;

// // //                 // Serial output ringkas kontinu
// // //                 Serial.print(F("[DATA] raw="));
// // //                 Serial.print(raw_ppm, 2);
// // //                 Serial.print(F(" ppm ("));
// // //                 Serial.print(ppmToPercentVol(raw_ppm), 4);
// // //                 Serial.print(F(" %Vol), corrected="));
// // //                 Serial.print(corrected_ppm, 2);
// // //                 Serial.print(F(" ppm ("));
// // //                 Serial.print(ppmToPercentVol(corrected_ppm), 4);
// // //                 Serial.println(F(" %Vol)"));

// // //                 Serial.print(F("       T="));
// // //                 Serial.print(scd30.temperature, 2);
// // //                 Serial.print(F(" C, RH="));
// // //                 Serial.print(scd30.relative_humidity, 2);
// // //                 Serial.println(F(" %"));
// // //             }
// // //             else
// // //             {
// // //                 Serial.println(F("[WARN] Gagal baca data SCD30"));
// // //             }
// // //         }
// // //         else
// // //         {
// // //             // belum ada sample baru dari sensor
// // //         }
// // //     }

// // //     // 2. Cek tombol kalibrasi (CAL_BUTTON_PIN aktif LOW)
// // //     if (now - lastButtonCheckMs >= BUTTON_DEBOUNCE_MS)
// // //     {
// // //         lastButtonCheckMs = now;
// // //         bool currentButtonState = digitalRead(CAL_BUTTON_PIN);

// // //         // Deteksi transisi HIGH -> LOW (tombol ditekan)
// // //         if (lastButtonState == HIGH && currentButtonState == LOW)
// // //         {
// // //             buttonWasPressed = true;
// // //         }
// // //         lastButtonState = currentButtonState;
// // //     }

// // //     if (buttonWasPressed)
// // //     {
// // //         buttonWasPressed = false;

// // //         if (!isnan(last_measured_ppm))
// // //         {
// // //             performCalibration(last_measured_ppm);
// // //         }
// // //         else
// // //         {
// // //             Serial.println(F("[INFO] Belum ada data sensor untuk kalibrasi tombol."));
// // //         }
// // //     }

// // //     // 3. Cek input Serial (command parser simple)
// // //     if (Serial.available())
// // //     {
// // //         String cmd = Serial.readStringUntil('\n');
// // //         cmd.trim(); // hapus spasi/CRLF

// // //         // Ubah ke uppercase untuk cek keyword awal
// // //         String upperCmd = cmd;
// // //         upperCmd.toUpperCase();

// // //         if (upperCmd.startsWith("REF"))
// // //         {
// // //             // Format: REF <angka>
// // //             // kita ambil substring setelah "REF"
// // //             int idx = cmd.indexOf(' ');
// // //             if (idx > 0 && (idx + 1) < (int)cmd.length())
// // //             {
// // //                 String valStr = cmd.substring(idx + 1);
// // //                 valStr.trim();

// // //                 float refPercent = valStr.toFloat(); // %Vol
// // //                 if (refPercent > 0.0f)
// // //                 {
// // //                     ref_target_ppm = percentVolToPPM(refPercent);
// // //                     saveCalibrationToNVS();
// // //                     Serial.println(F("[OK] Nilai referensi diset."));
// // //                     Serial.print(F("ref_target_ppm = "));
// // //                     Serial.print(ref_target_ppm, 2);
// // //                     Serial.print(F(" ppm ("));
// // //                     Serial.print(refPercent, 4);
// // //                     Serial.println(F(" %Vol)"));
// // //                 }
// // //                 else
// // //                 {
// // //                     Serial.println(F("[ERR] REF tidak valid. Contoh: REF 4.85"));
// // //                 }
// // //             }
// // //             else
// // //             {
// // //                 Serial.println(F("[ERR] Format REF salah. Contoh: REF 4.85"));
// // //             }
// // //         }

// // //         else if (upperCmd == "CAL")
// // //         {
// // //             if (!isnan(last_measured_ppm))
// // //             {
// // //                 performCalibration(last_measured_ppm);
// // //             }
// // //             else
// // //             {
// // //                 Serial.println(F("[INFO] Belum ada data sensor untuk kalibrasi."));
// // //             }
// // //         }

// // //         else if (upperCmd == "STATUS")
// // //         {
// // //             printStatus(last_measured_ppm, !isnan(last_measured_ppm));
// // //         }

// // //         else if (upperCmd == "RESETCAL")
// // //         {
// // //             co2_offset_ppm = 0.0f;
// // //             saveCalibrationToNVS();
// // //             Serial.println(F("[OK] Offset dikosongkan (0 ppm) dan disimpan."));
// // //         }

// // //         else
// // //         {
// // //             Serial.println(F("[?] Perintah tidak dikenal. Gunakan STATUS untuk bantuan."));
// // //         }
// // //     }

// // //     // kecilkan beban loop
// // //     delay(5);
// // // }

// // // nitip 3

// // #include <Arduino.h>
// // #include <Wire.h>
// // #include <Adafruit_SCD30.h>
// // #include <Preferences.h>

// // ///////////////////////
// // // PIN & CONSTANTS
// // ///////////////////////
// // #define I2C_SDA_PIN 21
// // #define I2C_SCL_PIN 22
// // #define CAL_BUTTON_PIN 18 // tombol kalibrasi, aktif LOW
// // #define BUTTON_DEBOUNCE_MS 200

// // // Interval pembacaan sensor (ms)
// // #define SAMPLE_INTERVAL_MS 5000

// // // Key di NVS
// // #define NVS_NAMESPACE "calib"
// // #define NVS_KEY_OFFSET "co2_off"
// // #define NVS_KEY_REFPPM "ref_ppm"

// // // Default nilai referensi untuk kalibrasi (dalam %Vol).
// // // Misal: udara bebas ~0.04 %Vol CO2 -> 400 ppm.
// // #define DEFAULT_REF_PERCENT 0.04f // 0.04%Vol = 400 ppm

// // ///////////////////////
// // // GLOBAL STATE
// // ///////////////////////
// // Adafruit_SCD30 scd30;
// // Preferences prefs;

// // float co2_offset_ppm = 0.0f; // offset kalibrasi (ppm), disimpan di NVS
// // float ref_target_ppm = 0.0f; // nilai referensi (ppm) yang alat pembanding katakan benar
// // unsigned long lastSampleMs = 0;
// // unsigned long lastButtonCheckMs = 0;
// // bool lastButtonState = HIGH; // kita pakai pull-up -> HIGH = idle
// // bool buttonWasPressed = false;

// // ///////////////////////
// // // UTILITIES
// // ///////////////////////

// // float ppmToPercentVol(float ppm)
// // {
// //     // 1%Vol = 10,000 ppm
// //     return ppm / 10000.0f;
// // }

// // float percentVolToPPM(float percentVol)
// // {
// //     return percentVol * 10000.0f;
// // }

// // void loadCalibrationFromNVS()
// // {
// //     prefs.begin(NVS_NAMESPACE, true); // read-only
// //     co2_offset_ppm = prefs.getFloat(NVS_KEY_OFFSET, 0.0f);
// //     ref_target_ppm = prefs.getFloat(NVS_KEY_REFPPM, percentVolToPPM(DEFAULT_REF_PERCENT));
// //     prefs.end();
// // }

// // void saveCalibrationToNVS()
// // {
// //     prefs.begin(NVS_NAMESPACE, false); // write
// //     prefs.putFloat(NVS_KEY_OFFSET, co2_offset_ppm);
// //     prefs.putFloat(NVS_KEY_REFPPM, ref_target_ppm);
// //     prefs.end();
// // }

// // // Hitung offset baru berdasar pembacaan sensor saat ini dan ref_target_ppm
// // // offset = ref - measured
// // // lalu simpan
// // void performCalibration(float measured_ppm)
// // {
// //     co2_offset_ppm = ref_target_ppm - measured_ppm;
// //     saveCalibrationToNVS();

// //     Serial.println(F("=== KALIBRASI DISIMPAN ==="));
// //     Serial.print(F("Measured (ppm): "));
// //     Serial.println(measured_ppm, 2);
// //     Serial.print(F("Reference (ppm): "));
// //     Serial.println(ref_target_ppm, 2);
// //     Serial.print(F("Offset baru (ppm): "));
// //     Serial.println(co2_offset_ppm, 2);
// //     Serial.println(F("=========================="));
// // }

// // // print status lengkap
// // void printStatus(float latest_meas_ppm, bool haveLatest)
// // {
// //     Serial.println(F("\n----- STATUS -----"));

// //     if (haveLatest)
// //     {
// //         float corrected_ppm = latest_meas_ppm + co2_offset_ppm;
// //         Serial.print(F("CO2 raw (ppm): "));
// //         Serial.println(latest_meas_ppm, 2);
// //         Serial.print(F("CO2 raw (%Vol): "));
// //         Serial.println(ppmToPercentVol(latest_meas_ppm), 4);

// //         Serial.print(F("CO2 corrected (ppm): "));
// //         Serial.println(corrected_ppm, 2);
// //         Serial.print(F("CO2 corrected (%Vol): "));
// //         Serial.println(ppmToPercentVol(corrected_ppm), 4);
// //     }
// //     else
// //     {
// //         Serial.println(F("Belum ada data sensor terbaru."));
// //     }

// //     Serial.print(F("ref_target_ppm (ppm): "));
// //     Serial.println(ref_target_ppm, 2);
// //     Serial.print(F("ref_target (%Vol): "));
// //     Serial.println(ppmToPercentVol(ref_target_ppm), 4);

// //     Serial.print(F("offset sekarang (ppm): "));
// //     Serial.println(co2_offset_ppm, 2);
// //     Serial.print(F("offset sekarang (%Vol): "));
// //     Serial.println(ppmToPercentVol(co2_offset_ppm), 4);

// //     Serial.println(F("Perintah Serial yang tersedia:"));
// //     Serial.println(F("  REF <angka_percentVol>  -> set nilai referensi (contoh: REF 0.04)"));
// //     Serial.println(F("  CAL                     -> kalibrasi paksa ke 400 ppm"));
// //     Serial.println(F("  STATUS                  -> tampilkan status ini"));
// //     Serial.println(F("  RESETCAL                -> hapus offset jadi 0 & simpan"));
// //     Serial.println(F("--------------------\n"));
// // }

// // ///////////////////////
// // // SETUP
// // ///////////////////////
// // void setup()
// // {
// //     Serial.begin(115200);
// //     delay(1000);

// //     // I2C init
// //     Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

// //     // Tombol kalibrasi
// //     pinMode(CAL_BUTTON_PIN, INPUT_PULLUP);

// //     // Load offset dan ref dari NVS
// //     loadCalibrationFromNVS();

// //     // Init sensor SCD30
// //     if (!scd30.begin())
// //     {
// //         Serial.println(F("ERROR: SCD30 tidak terdeteksi. Cek wiring/power!"));
// //         while (1)
// //         {
// //             delay(1000);
// //         }
// //     }

// //     // Set interval pengukuran (detik). Min 2 detik.
// //     if (!scd30.setMeasurementInterval(2))
// //     {
// //         Serial.println(F("Gagal set interval, pakai default internal sensor."));
// //     }

// //     // Matikan auto self calibration
// //     scd30.selfCalibrationEnabled(false);

// //     Serial.println(F("SCD30 inisialisasi OK."));
// //     Serial.println(F("System siap.\nKetik STATUS di Serial Monitor untuk lihat info.\n"));
// // }

// // ///////////////////////
// // // LOOP
// // ///////////////////////
// // void loop()
// // {
// //     static float last_measured_ppm = NAN;
// //     bool haveLatest = false;

// //     // 1. Cek data sensor per SAMPLE_INTERVAL_MS
// //     unsigned long now = millis();
// //     if (now - lastSampleMs >= SAMPLE_INTERVAL_MS)
// //     {
// //         lastSampleMs = now;

// //         if (scd30.dataReady())
// //         {
// //             if (scd30.read())
// //             {
// //                 float raw_ppm = scd30.CO2; // ppm dari library
// //                 float corrected_ppm = raw_ppm + co2_offset_ppm;

// //                 last_measured_ppm = raw_ppm;
// //                 haveLatest = true;

// //                 // Serial output ringkas kontinu
// //                 Serial.print(F("[DATA] raw="));
// //                 Serial.print(raw_ppm, 2);
// //                 Serial.print(F(" ppm ("));
// //                 Serial.print(ppmToPercentVol(raw_ppm), 4);
// //                 Serial.print(F(" %Vol), corrected="));
// //                 Serial.print(corrected_ppm, 2);
// //                 Serial.print(F(" ppm ("));
// //                 Serial.print(ppmToPercentVol(corrected_ppm), 4);
// //                 Serial.println(F(" %Vol)"));

// //                 Serial.print(F("       T="));
// //                 Serial.print(scd30.temperature, 2);
// //                 Serial.print(F(" C, RH="));
// //                 Serial.print(scd30.relative_humidity, 2);
// //                 Serial.println(F(" %"));
// //             }
// //             else
// //             {
// //                 Serial.println(F("[WARN] Gagal baca data SCD30"));
// //             }
// //         }
// //     }

// //     // 2. Cek tombol kalibrasi (CAL_BUTTON_PIN aktif LOW)
// //     if (now - lastButtonCheckMs >= BUTTON_DEBOUNCE_MS)
// //     {
// //         lastButtonCheckMs = now;
// //         bool currentButtonState = digitalRead(CAL_BUTTON_PIN);

// //         // Deteksi transisi HIGH -> LOW (tombol ditekan)
// //         if (lastButtonState == HIGH && currentButtonState == LOW)
// //         {
// //             buttonWasPressed = true;
// //         }
// //         lastButtonState = currentButtonState;
// //     }

// //     if (buttonWasPressed)
// //     {
// //         buttonWasPressed = false;

// //         if (!isnan(last_measured_ppm))
// //         {
// //             ref_target_ppm = 400.0f; // paksa kalibrasi ke 400 ppm
// //             performCalibration(last_measured_ppm);
// //         }
// //         else
// //         {
// //             Serial.println(F("[INFO] Belum ada data sensor untuk kalibrasi tombol."));
// //         }
// //     }

// //     // 3. Cek input Serial (command parser simple)
// //     if (Serial.available())
// //     {
// //         String cmd = Serial.readStringUntil('\n');
// //         cmd.trim();

// //         String upperCmd = cmd;
// //         upperCmd.toUpperCase();

// //         if (upperCmd.startsWith("REF"))
// //         {
// //             int idx = cmd.indexOf(' ');
// //             if (idx > 0 && (idx + 1) < (int)cmd.length())
// //             {
// //                 String valStr = cmd.substring(idx + 1);
// //                 valStr.trim();

// //                 float refPercent = valStr.toFloat();
// //                 if (refPercent > 0.0f)
// //                 {
// //                     ref_target_ppm = percentVolToPPM(refPercent);
// //                     saveCalibrationToNVS();
// //                     Serial.println(F("[OK] Nilai referensi diset."));
// //                     Serial.print(F("ref_target_ppm = "));
// //                     Serial.print(ref_target_ppm, 2);
// //                     Serial.print(F(" ppm ("));
// //                     Serial.print(refPercent, 4);
// //                     Serial.println(F(" %Vol)"));
// //                 }
// //                 else
// //                 {
// //                     Serial.println(F("[ERR] REF tidak valid. Contoh: REF 0.04"));
// //                 }
// //             }
// //             else
// //             {
// //                 Serial.println(F("[ERR] Format REF salah. Contoh: REF 0.04"));
// //             }
// //         }

// //         else if (upperCmd == "CAL")
// //         {
// //             if (!isnan(last_measured_ppm))
// //             {
// //                 ref_target_ppm = 400.0f; // paksa kalibrasi ke 400 ppm
// //                 performCalibration(last_measured_ppm);
// //             }
// //             else
// //             {
// //                 Serial.println(F("[INFO] Belum ada data sensor untuk kalibrasi."));
// //             }
// //         }

// //         else if (upperCmd == "STATUS")
// //         {
// //             printStatus(last_measured_ppm, !isnan(last_measured_ppm));
// //         }

// //         else if (upperCmd == "RESETCAL")
// //         {
// //             co2_offset_ppm = 0.0f;
// //             saveCalibrationToNVS();
// //             Serial.println(F("[OK] Offset dikosongkan (0 ppm) dan disimpan."));
// //         }

// //         else
// //         {
// //             Serial.println(F("[?] Perintah tidak dikenal. Gunakan STATUS untuk bantuan."));
// //         }
// //     }

// //     delay(5);
// // }

// //nitip 4 kalibrasi terakhir
// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_SCD30.h>

// ///////////////////////
// // PIN & CONSTANTS
// ///////////////////////
// #define I2C_SDA_PIN 21
// #define I2C_SCL_PIN 22
// #define SCD30_I2C_ADDR 0x61
// #define MEAS_INTERVAL_SEC 3 // interval pengukuran di dalam sensor (detik)

// // Global object
// Adafruit_SCD30 scd30;

// ///////////////////////
// // I2C / SENSOR CHECK
// ///////////////////////
// bool checkI2C_SCD30()
// {
//     Wire.beginTransmission(SCD30_I2C_ADDR);
//     uint8_t error = Wire.endTransmission();

//     if (error == 0)
//     {
//         Serial.println(F("[I2C] SCD30 terdeteksi di alamat 0x61."));
//         return true;
//     }
//     else
//     {
//         Serial.print(F("[I2C] Gagal komunikasi dengan SCD30. Error code: "));
//         Serial.println(error);
//         return false;
//     }
// }

// ///////////////////////
// // UTILITIES
// ///////////////////////
// void printHelp()
// {
//     Serial.println(F("Perintah Serial yang tersedia:"));
//     Serial.println(F("  HELP        -> tampilkan bantuan ini"));
//     Serial.println(F("  STATUS      -> tampilkan status/config SCD30"));
//     Serial.println(F("  FRC 400     -> Forced Re-Calibration ke 400 ppm (fresh air)"));
//     Serial.println(F("  FRC <ppm>   -> FRC ke nilai lain, misal: FRC 800"));
//     Serial.println(F("  ASC ON      -> enable Automatic Self-Calibration"));
//     Serial.println(F("  ASC OFF     -> disable Automatic Self-Calibration"));
//     Serial.println();
// }

// void printStatus()
// {
//     Serial.println();
//     Serial.println(F("===== STATUS SCD30 ====="));

//     // Interval pengukuran
//     uint16_t interval = scd30.getMeasurementInterval();
//     Serial.print(F("Measurement interval : "));
//     Serial.print(interval);
//     Serial.println(F(" detik"));

//     // Forced Recalibration reference
//     uint16_t frcRef = scd30.getForcedCalibrationReference();
//     Serial.print(F("FRC reference        : "));
//     Serial.print(frcRef);
//     Serial.println(F(" ppm"));

//     // ASC status
//     bool asc = scd30.selfCalibrationEnabled();
//     Serial.print(F("ASC (auto calib)     : "));
//     Serial.println(asc ? F("ENABLED") : F("DISABLED"));

//     // Altitude offset
//     uint16_t alt = scd30.getAltitudeOffset();
//     Serial.print(F("Altitude offset      : "));
//     Serial.print(alt);
//     Serial.println(F(" meter"));

//     // Temperature offset
//     float toffset = scd30.getTemperatureOffset() / 100.0f;
//     Serial.print(F("Temp offset          : "));
//     Serial.print(toffset, 2);
//     Serial.println(F(" C"));

//     Serial.println(F("========================"));
//     Serial.println();
// }

// void handleSerialCommands()
// {
//     if (!Serial.available())
//         return;

//     String line = Serial.readStringUntil('\n');
//     line.trim();
//     if (line.length() == 0)
//         return;

//     // Buat versi uppercase untuk cek keyword
//     String upper = line;
//     upper.toUpperCase();

//     if (upper == "HELP")
//     {
//         printHelp();
//     }
//     else if (upper == "STATUS")
//     {
//         printStatus();
//     }
//     else if (upper.startsWith("FRC"))
//     {
//         // Format: "FRC 400" atau "FRC 800"
//         int sp = line.indexOf(' ');
//         if (sp > 0 && sp + 1 < (int)line.length())
//         {
//             String valStr = line.substring(sp + 1);
//             valStr.trim();
//             int refPPM = valStr.toInt();

//             if (refPPM >= 400 && refPPM <= 2000)
//             {
//                 Serial.print(F("[CMD] FRC "));
//                 Serial.print(refPPM);
//                 Serial.println(F(" ppm"));

//                 if (scd30.forceRecalibrationWithReference((uint16_t)refPPM))
//                 {
//                     Serial.println(F("[OK] Forced Re-Calibration berhasil."));
//                     Serial.print(F("     FRC reference sekarang: "));
//                     Serial.print(scd30.getForcedCalibrationReference());
//                     Serial.println(F(" ppm"));
//                 }
//                 else
//                 {
//                     Serial.println(F("[ERR] Gagal melakukan FRC (forceRecalibrationWithReference)."));
//                 }
//             }
//             else
//             {
//                 Serial.println(F("[ERR] Nilai FRC harus 400 - 2000 ppm. Contoh: FRC 400"));
//             }
//         }
//         else
//         {
//             Serial.println(F("[ERR] Format FRC salah. Contoh: FRC 400"));
//         }
//     }
//     else if (upper == "ASC ON")
//     {
//         if (scd30.selfCalibrationEnabled(true))
//         {
//             Serial.println(F("[OK] ASC di-ENABLE."));
//         }
//         else
//         {
//             Serial.println(F("[ERR] Gagal mengubah status ASC."));
//         }
//     }
//     else if (upper == "ASC OFF")
//     {
//         if (scd30.selfCalibrationEnabled(false))
//         {
//             Serial.println(F("[OK] ASC di-DISABLE."));
//         }
//         else
//         {
//             Serial.println(F("[ERR] Gagal mengubah status ASC."));
//         }
//     }
//     else
//     {
//         Serial.print(F("[?] Perintah tidak dikenal: "));
//         Serial.println(line);
//         printHelp();
//     }
// }

// ///////////////////////
// // SETUP
// ///////////////////////
// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     Serial.println();
//     Serial.println(F("==================================="));
//     Serial.println(F("  TES SEDERHANA SENSOR SCD30"));
//     Serial.println(F("==================================="));

//     // Inisialisasi I2C
//     Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
//     Serial.print(F("[INIT] I2C mulai di SDA="));
//     Serial.print(I2C_SDA_PIN);
//     Serial.print(F(", SCL="));
//     Serial.println(I2C_SCL_PIN);

//     // Cek I2C
//     if (!checkI2C_SCD30())
//     {
//         Serial.println(F("[ERROR] SCD30 tidak terdeteksi. Cek wiring/power!"));
//         while (1)
//         {
//             delay(1000); // stop di sini kalau I2C gagal
//         }
//     }

//     // Inisialisasi SCD30
//     if (!scd30.begin())
//     {
//         Serial.println(F("[ERROR] Inisialisasi SCD30 gagal (scd30.begin() return false)."));
//         while (1)
//         {
//             delay(1000); // stop di sini kalau init gagal
//         }
//     }

//     // Set interval pengukuran di dalam sensor (min 2 detik)
//     if (scd30.setMeasurementInterval(MEAS_INTERVAL_SEC))
//     {
//         Serial.print(F("[INIT] Interval pengukuran diset ke "));
//         Serial.print(MEAS_INTERVAL_SEC);
//         Serial.println(F(" detik."));
//     }
//     else
//     {
//         Serial.println(F("[WARN] Gagal set interval pengukuran, pakai default sensor."));
//     }

//     // Matikan ASC dulu supaya FRC tidak langsung dioverride
//     if (scd30.selfCalibrationEnabled(false))
//     {
//         Serial.println(F("[INIT] ASC dimatikan (self calibration OFF)."));
//     }
//     else
//     {
//         Serial.println(F("[WARN] Gagal mematikan ASC (self calibration)."));
//     }

//     Serial.println(F("[OK] SCD30 inisialisasi berhasil."));
//     Serial.println(F("[INFO] Sensor akan mengukur CO2 setiap beberapa detik."));
//     Serial.println();
//     printHelp();
//     printStatus();
// }

// ///////////////////////
// // LOOP
// ///////////////////////
// void loop()
// {
//     // 1) Baca data kalau sudah ready
//     if (scd30.dataReady())
//     {
//         if (scd30.read())
//         {
//             float co2 = scd30.CO2;              // ppm
//             float temp = scd30.temperature;     // °C
//             float rh = scd30.relative_humidity; // %

//             Serial.print(F("[DATA] CO2 = "));
//             Serial.print(co2, 2);
//             Serial.println(F(" ppm"));

//             // Tampilkan temperatur & RH hanya kalau temperatur > 30°C
//             if (temp > 30.0f)
//             {
//                 Serial.print(F("       T = "));
//                 Serial.print(temp, 2);
//                 Serial.print(F(" C, RH = "));
//                 Serial.print(rh, 2);
//                 Serial.println(F(" %"));
//             }

//             Serial.println();
//         }
//         else
//         {
//             Serial.println(F("[WARN] Gagal baca data dari SCD30."));
//         }
//     }

//     // 2) Cek command dari Serial
//     handleSerialCommands();

//     // kecilkan beban CPU & traffic I2C
//     delay(100);
// }
