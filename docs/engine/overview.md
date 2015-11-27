# Engine

Engine je svým způsobem virtualní parní stroj který se skládá z mnoha různých subsystému např. `render system`,
[log system][log_system],`resource manager`, `console server` atd. Samotné jádro enginu je naprogramováno v C/C++
(resp. v C++ ale stylem "Céčkové školy") a v [LUA][lua]. A to všechno se stará o to aby jsme
mohli [zabíjet haldy emzáku][doom], [hackovat kamery pod vodou][bioshock], [řádit v radiaci][fallout],
[táhnout se za záchanou lidstva][spellcross], [hrdině osvobozovat svět][had] nebo [začít politickou karieru][mafie].

---

[doom]: https://cs.wikipedia.org/wiki/Doom
[cac]: https://en.wikipedia.org/wiki/Command_%26_Conquer_%281995_video_game%29
[residentevil]: https://cs.wikipedia.org/wiki/Resident_Evil
[bioshock]: https://cs.wikipedia.org/wiki/BioShock
[fallout]: https://cs.wikipedia.org/wiki/Fallout
[spellcross]: https://cs.wikipedia.org/wiki/Spellcross
[had]: https://cs.wikipedia.org/wiki/Hidden_%26_Dangerous
[mafie]: https://cs.wikipedia.org/wiki/Mafia:_The_City_of_Lost_Heaven
[lua]: http://www.lua.org/

[log_system]: api/cpp/log.md