select * from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s2.matrnr=s1.matrnr and s1.name = 'Schopenhauer'
select titel from vorlesungen v, studenten s, hoeren h where s.matrnr=h.matrnr and v.vorlnr = h.vorlnr and s.semester = 2
select * from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s1.name = 'Schopenhauer'
select vorlnr, titel from vorlesungen v where v.vorlnr = 5041
select vorlnr, titel from vorlesungen v where v.titel = 'Logik'
select vorlnr, titel from vorlesungen v where v.sws = 4
select vorlnr, titel from vorlesungen v where v.vorlnr = 5041 and v.vorlnr = 5031
select vorlnr, titel, persnr from vorlesungen v where v.vorlnr = 5041
select vorlnr, titel from vorlesungen v where v.vorlnr = 5041 and v.persnr = 5
select titel from vorlesungen titel where titel.titel = 'Ethik'
select matrnr from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s2.matrnr = s1.matrnr and s1.name = 'Schopenhauer'
select vorlnr titel from vorlesungen v where v.vorlnr = 5041
select, vorlnr from vorlesungen v where v.vorlnr = 5041
select vorlnr, titel from vorlesungen v where v.vorlnr 5041
select vorlnr,,,, titel from vorlesungen v where v.vorlnr 5041
select vorlnr, titel f vorlesungen v where v.vorlnr = 5041
select from vorlesungen v where v.vorlnr = 5041
select name, titel from vorlesungen v, studenten s, hoeren h where s.matrnr=h.matrnr and v.vorlnr = h.vorlnr
select name, titel from vorlesungen v, professoren p where p.persnr=v.gelesenvon and p.rang='C4' and v.sws=3
select raum, note from pruefen p, assistenten a, professoren prof where a.boss=p.persnr and prof.persnr=a.boss and p.persnr=prof.persnr and note=2
