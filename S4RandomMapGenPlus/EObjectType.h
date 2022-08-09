#pragma once
#include "pch.h"
#include <cstdint>
enum class EObjectType : uint8_t{
	Eiche = 1U,
	Buche,
	Esche,
	Linde,
	Birke,
	Silberpappel,
	Kastanie,
	Ahorn,
	Tanne,
	Fichte,
	Kokospalme,
	Dattelpalme,
	Walnuss,
	Korkeiche,
	Pinie,
	GemeinePinie,
	GrosserOlivenbaum,
	KleinerOlivenbaum,
	TropischeDattelpalme1,
	TropischeDattelpalme2,
	DunkelblaettrigeTropischePalme,
	TropischePalme,
	DunkleKastanie,
	DunkleEiche,
	DunklerAhorn,
	DunkleBuche,
	DunkleWalnuss,
	DunkleKorkeiche,
	DunkleTanne,
	NO_OBJECT0,
	DunklerDattelbaum,
	NO_OBJECT1,
	NO_OBJECT2,
	NO_OBJECT3,
	NO_OBJECT4,
	NO_OBJECT5,
	NO_OBJECT6,
	NO_OBJECT7,
	NO_OBJECT8,
	NO_OBJECT9,
	ZerklueftetesRiff,
	VerwildertesTrojanischesPferd,
	Ginsterbusch,
	Magnolie,
	Haselnuss,
	Holunder,
	Liguster,
	Weissdorn,
	Preiselbeere,
	HoherKaktus,
	KeltischesKreuz,
	Kuhskelett,
	MenschlichesSkelett,
	Wagenwrack,
	Schiffswrack,
	BraunerSteinGroesse4,
	BraunerSteinGroesse7,
	BraunerSteinGroesse5,
	BraunerSteinGroesse8,
	BraunerSteinGroesse3,
	BraunerSteinGroesse9,
	BraunerSteinGroesse1,
	BraunerSteinGroesse10,
	BraunerSteinGroesse2,
	BraunerSteinGroesse6,
	GrauerSteinGroesse6,
	GrauerSteinGroesse2,
	GrauerSteinGroesse10,
	GrauerSteinGroesse5,
	GrauerSteinGroesse3,
	GrauerSteinGroesse4,
	GrauerSteinGroesse9,
	GrauerSteinGroesse7,
	GrauerSteinGroesse1,
	GrauerSteinGroesse8,
	Nesseln,
	HellesGras,
	Dickblatt,
	Loewenzahn,
	Huflattich,
	Minze,
	Beifuss,
	Hirschzunge,
	Sauerampfer,
	Distel,
	NO_OBJECT10,
	Brunnenruine,
	NO_OBJECT11,
	Tuempel,
	Ruine,
	Vogelscheuche,
	Pilzkreis,
	RoetlicherStein,
	KleinerdunklerFels,
	RoetlicherFels,
	GrosserroetlicherFels,
	MittlererroetlicherFels,
	KleinerroetlicherFels,
	DunkleFelsen,
	MittlererdunklerFels,
	GrosserdunklerFels,
	KleinerDunklerStein,
	DunklesEfeu,
	BoeserFliegenpilz,
	BoeserSteinpilz,
	BoeserChampignon,
	SpitzesRiff,
	BreitesRiff,
	KleinesRiff,
	ZerklueftetesRiff2,
	DunkleHaselnuss,
	unklerLiguster,
	DunkleMagnolie,
	DunklePreiselbeere,
	MorbusDunklerStatuenboden,
	Saeule1,
	TrojanischesPferdimBau,
	TrojanischesPferd,
	Portal,
	GrosserKaktus,
	MittlererKaktus,
	KleinerKaktus,
	HolzstoeckeamRandbeiderzeitigkonstruiertenGebaeuden,
	SteinezumBauen1,
	SteinezumBauen2,
	SteinezumBauen3,
	SteinezumBauen4,
	SteinezumBauen5,
	SteinezumBauen6,
	SteinezumBauen7,
	SteinezumBauen8,
	SteinezumBauen9,
	SteinezumBauen10,
	SteinezumBauen11,
	SteinezumBauen12,
	SteinezumBauen13,
	DunkleSteinezumBauen1,
	DunkleSteinezumBauen2,
	DunkleSteinezumBauen3,
	DunkleSteinezumBauen4,
	DunkleSteinezumBauen5,
	DunkleSteinezumBauen6,
	DunkleSteinezumBauen7,
	DunkleSteinezumBauen8,
	DunkleSteinezumBauen9,
	DunkleSteinezumBauen10,
	DunkleSteinezumBauen11,
	DunkleSteinezumBauen12,
	DunkleSteinezumBauen13,
	DunklerTuempel,
	DunklerSchneemann,
	Saeule2,
	Saeulenecke1,
	Saeulenecke2,
	Saeulenecke3,
	Saeulenecke4,
	UmgefalleneSaeule,
	HalbeSaeule,
	HalbUmgefalleneSaeule,
	Maulbeere,
	Grab,
	AusgereifterWeinstock,
	Schilfgras,
	Moorbeere,
	Pampasgras,
	Schilfrohrklein,
	Schilfrohrmittel,
	Schilfrohrgross,
	Muschelgross,
	Muschelklein,
	RoteSeerose,
	WeisseSeerose,
	GelbeSeerose,
	TrockeneZweige,
	TrockenerZweig,
	TrockenerAst,
	TrockenerBusch,
	HalberKleinerRinderhaufen,
	Wuestenbusch,
	Wuestenstrauch,
	Fliegenpilz,
	Steinpilz,
	Champignon,
	BlumenweissLila,
	Blumenweiss,
	Blumenlila,
	BlumenweissRosa,
	Blumenrosa,
	KleinerRinderhaufen,
	MittlererRinderhaufen,
	GrosserRinderhaufen,
	Saeulenreihe1,
	Schild,
	KohlevorkommenSchild1,
	KohlevorkommenSchild2,
	KohlevorkommenSchild3,
	GoldvorkommenSchild1,
	GoldvorkommenSchild2,
	GoldvorkommenSchild3,
	EisenvorkommenSchild1,
	EisenvorkommenSchild2,
	EisenvorkommenSchild3,
	SteinvorkommenSchild1,
	SteinvorkommenSchild2,
	SteinvorkommenSchild3,
	SchwefelvorkommenSchild1,
	SchwefelvorkommenSchild2,
	SchwefelvorkommenSchild3,
	ueberreifesGetreidefeld,
	ReifesGetreidefeld,
	Agave,
	DunklesVolkreiferPilz,
	DunklesVolkhalbreiferPilz,
	DunklesVolknichtreiferPilz,
	Schneemann,
	// Interactive with SGround
	DunklesLandKonvertierungsEffekt,
	Bienennest,
	Saeulenreihe2,
	Saeulenbogen1,
	Saeulenbogen2,
	MagischesKraut,
	Pfaffenhuetchen,
	Schloss ,
	GrosserTurmmitFeueraufderSpitze ,
	RoemischeFreiheitsstatue ,
	MosaikStatue ,
	MosaikStatueimBau ,
	MAMAGebaeude ,
	NO_OBJECT12,
	EinbesondererBuschausdemDieneueWeltAddOn,
	TotengrabmitSpinnennetzenundKnochenschaedel,
	PRunenpyramide,
	DunklerSteinGroesse1,
	DunklerSteinGroesse2,
	DunklerSteinGroesse3,
	DunklerSteinGroesse4,
	DunklerSteinGroesse5,
	DunklerSteinGroesse6,
	DunklerSteinGroesse7,
	DunklerSteinGroesse8,
	DunklerSteinGroesse9,
	DunklerSteinGroesse10,
	DunklerroterSteinGroesse1,
	DunklerroterSteinGroesse2,
	DunklerroterSteinGroesse3,
	DunklerroterSteinGroesse4,
	DunklerroterSteinGroesse5,
	DunklerroterSteinGroesse6,
	DunklerroterSteinGroesse7,
	DunklerroterSteinGroesse8,
	DunklerroterSteinGroesse9,
	DunklerroterSteinGroesse10,
	SpitzesRiff2,
	DunkleSpuckpflanze,
	Sonnenblumen
		/* NOT OBJECTS, EFFECTS ,
	Wasserwellen1 ,
	Wasserwellen2 ,
	Wasserwellen3 ,
	Wasserwellen4 ,
	Wasserwellen5 ,
	Wasserwellen6 ,
	Wasserwellen7 ,
	Wasserwellen8 ,
	Wasserwellen9 ,
	Wasserwellen10 ,
	Wasserwellen11 ,
	Wasserwellen12 ,
	Wasserwellen13 ,
	Wasserwellen14 ,
	Wasserwellen15 ,
	Wasserwellen16 ,
	Grenzpunkte,
	GoldBarren ,
	EisenBarren ,
	UnidentifizierbaresObjekt,
	Stein ,
	UnidentifizierbaresObjekt2,
	Fisch ,
	StartpositionsFlagge ,
	SteinschildTYPA ,
	SteinschildTYPB ,
	SteinschildTYPC ,
	KleineHaeuserflaggerot,
	KleineHaeuserflaggeblau,
	KleineHaeuserflaggegruen,
	KleineHaeuserflaggegelb,
	KleineHaeuserflaggeviolett,
	KleineHaeuserflaggeockergelb,
	KleineHaeuserflaggetuerkis,
	KleineHaeuserflaggeweiss*/
};