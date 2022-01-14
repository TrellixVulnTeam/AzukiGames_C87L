#include "stdafx.h"
#include <random>
#include "BackGround.h"
#include "player/Player.h"
#include "item/Item.h"
#include "Door.h"
#include "enemy/firstWinEnemy/FirstWinEnemy.h"
#include "enemy/goteWinEnemy/GoteWinEnemy.h"
#include "enemy/boss/Boss.h"


namespace nsMyGame {

	nsLight::CDirectionLight* CBackGround::m_dirLight[2] = { nullptr };

	bool CBackGround::Start() {

		//プレイヤーを検索。
		auto player = FindGO<nsPlayer::CPlayer>(c_classNamePlayer);
		
		////プレイヤーを中心とするポイントライトを作成。
		//m_pointLight.push_back(NewGO<nsLight::CPointLight>(enPriority_Zeroth));
		//m_pointLight[0]->SetPosition(player->GetPosition());
		//m_pointLight[0]->SetColor({ 1.0f,1.0f,1.0f });
		//m_pointLight[0]->SetRange(300.0f);
		//m_pointLight[0]->SetAffectPowParam(2.5f);

		//ディレクションライトを作成。
		CreateDirLight();

		//m_modelRender = NewGO<CModelRender>(0);
		//m_modelRender->SetFilePathTkm("Assets/modelData/backGround/testStage.tkm");
		//m_modelRender->Init();
		//m_physicsStaticObject.CreateFromModel(
		//	*m_modelRender->GetModel(),
		//	m_modelRender->GetModel()->GetWorldMatrix()
		//);
		//m_physicsStaticObject.SetFriction(10.0f);

		//ステージをロード。
		LoadStage();

		return true;
	}

	void CBackGround::OnDestroy() {

		for (int i = 0; i < m_pointLightNum; i++) {

			DeleteGO(m_pointLight[i]);
			//m_pointLight[i] = nullptr;
		}
		m_pointLight.clear();

		for (int i = 0; i < m_doorNum; i++) {

			DeleteGO(m_door[i]);
			//m_door[i] = nullptr;
		}
		m_door.clear();

		for (int i = 0; i < m_fEnemyNum; i++) {

			DeleteGO(m_fWEnemy[i]);
			//m_fWEnemy[i] = nullptr;
		}
		m_fWEnemy.clear();

		for (int i = 0; i < m_gEnemyNum; i++) {

			DeleteGO(m_gWEnemy[i]);
			//m_gWEnemy[i] = nullptr;
		}
		m_gWEnemy.clear();

		for (int i = 0; i < m_itemNum; i++) {

			DeleteGO(m_item[i]);
			//m_item[i] = nullptr;
		}
		m_item.clear();

		RemoveDirLight();

		DeleteGO(m_boss);
	}

	void CBackGround::Update() {

		//プレイヤーを検索。
		auto player = FindGO<nsPlayer::CPlayer>(c_classNamePlayer);

		//	//プレイヤーの座標を取得。
		CVector3 playerPos = player->GetPosition();

		//規定値より100.0f大きい数を初期値とする。
		float vecToPlayerLength = c_distanceForOpenDoor + 100.0f;

		/*----------プレイヤーの選択状態を更新するための処理----------*/

		//リストに含まれるオブジェクトを参照。
		for (const auto& obj : m_door) {

			//クラスの名前を調べる。
			const std::type_info& typeInfo = typeid(*obj);

			//ドアだったら
			if (typeInfo == typeid(CDoor)) {

				//ドアクラスにキャスト。
				auto doorObj = dynamic_cast<CDoor*>(obj);

				//プレイヤーに伸びるベクトルを計算。
				CVector3 vecToPlayer = playerPos - doorObj->GetPosition();

				//プレイヤーとの最短距離を更新。
				if (vecToPlayer.Length() < vecToPlayerLength) {

					vecToPlayerLength = vecToPlayer.Length();

					//最短距離を更新したドアとプレイヤーの距離が一定以下かつ、そのドアが開いていないなら
					if (vecToPlayerLength <= c_distanceForOpenDoor && !doorObj->IsOpened()) {

						//プレイヤーを選択状態にする。
						player->SetSelectFlag(true);
						break;
					}
				}
				//それ以外は、プレイヤーは何も選択していない状態。
				else {

					//player->SetSelectFlag(false);
				}
			}
		}

		/*------------------------------------------------------------*/

		if (!m_createBoss) {
			//剛体との当たり判定を調べる。
			CPhysicsWorld::GetInstance()->ContactTest(player->GetCharacterController(), [&](const btCollisionObject& contactObject) {

				//トリガーボックスと接触した。
				if (m_noticePlayerTriggerBox.IsSelf(contactObject)) {

					//ボスを出現させる。
					m_boss = NewGO<nsEnemy::CBoss>(enPriority_Zeroth, c_classNameBoss);
					m_boss->SetPosition(m_bossPosition);
					m_boss->SetRotation(m_bossRotation);

					m_createBoss = true;
				}
			});
		}

		//プレイヤー中心のポイントライトの座標を更新。
		CVector3 playerLightPosition = player->GetPosition();
		playerLightPosition.y += 130.0f;
		m_pointLight[0]->SetPosition(playerLightPosition);

		//カメラの前方向に向けて放たれるディレクションライトの向きを更新。
		m_dirLight[1]->SetLigDirection(g_camera3D->GetForward());

		m_fireTime += GameTime().GameTimeFunc().GetFrameDeltaTime();

		//1.0秒ごとにエフェクトを再生。
		if (m_fireTime >= 1.0f) {

			for (const auto& fireEffect : m_fireEffect) {

				fireEffect->Play();
			}
			m_fireTime = 0.0f;
		}
	}
	void CBackGround::LoadStage() {

		//プレイヤーを検索。
		auto player = FindGO<nsPlayer::CPlayer>(c_classNamePlayer);

		//プレイヤーを中心とするポイントライトを作成。
		m_pointLight.push_back(NewGO<nsLight::CPointLight>(enPriority_Zeroth));
		m_pointLight[m_pointLightNum]->SetPosition(player->GetPosition());
		m_pointLight[m_pointLightNum]->SetColor({ 1.0f,1.0f,1.0f });
		m_pointLight[m_pointLightNum]->SetRange(300.0f);
		m_pointLight[m_pointLightNum]->SetAffectPowParam(2.5f);
		m_pointLightNum++;

		//ステージをロード。
		m_level.Init("Assets/level/stage_1.tkl", [&](LevelObjectData& objData) {

			if (objData.EqualObjectName("door")) {

				m_door.push_back(NewGO<CDoor>(enPriority_Zeroth));
				m_door[m_doorNum]->SetPosition(objData.position);
				m_door[m_doorNum]->SetRotation(objData.rotation);
				m_door[m_doorNum]->SetScale(objData.scale);
				m_doorNum++;
				return true;
			}

			if (objData.EqualObjectName("door_Lock")) {

				m_door.push_back(NewGO<CDoor>(enPriority_Zeroth));
				m_door[m_doorNum]->SetPosition(objData.position);
				m_door[m_doorNum]->SetRotation(objData.rotation);
				m_door[m_doorNum]->SetScale(objData.scale);

				//鍵をかける。
				m_door[m_doorNum]->Lock();
				m_doorNum++;
				return true;
			}

			if (objData.EqualObjectName("doorObj")) {

				m_door.push_back(NewGO<CDoor>(enPriority_Zeroth));
				m_door[m_doorNum]->SetPosition(objData.position);
				m_door[m_doorNum]->SetRotation(objData.rotation);
				m_door[m_doorNum]->SetScale(objData.scale);
				m_door[m_doorNum]->SetObj(true);
				m_doorNum++;
				return true;
			}

			if (objData.EqualObjectName("FEnemy")) {

				m_fWEnemy.push_back(NewGO<nsEnemy::CFirstWinEnemy>(enPriority_Zeroth, c_classNameEnemy));
				m_fWEnemy[m_fEnemyNum]->SetPosition(objData.position);
				m_fWEnemy[m_fEnemyNum]->SetRotation(objData.rotation);
				m_fEnemyNum++;
				return true;
			}

			if (objData.EqualObjectName("GEnemy")) {

				//m_gWEnemy.push_back(NewGO<nsEnemy::CGoteWinEnemy>(enPriority_Zeroth, c_classNameEnemy));
				//m_gWEnemy[gEnemyNum]->SetPosition(objData.position);
				//m_gWEnemy[gEnemyNum]->SetRotation(objData.rotation);
				//gEnemyNum++;
				return true;
			}

			if (objData.EqualObjectName("Boss")) {

				m_bossPosition = objData.position;
				m_bossRotation = objData.rotation;

				//ボスが登場するためのトリガーボックスを設定。
				m_noticePlayerTriggerBox.CreateBox(objData.position, CQuaternion::Identity, { 1300.0f,3000.0f,1300.0f });

				return true;
			}

			if (objData.EqualObjectName("key")) {

				m_item.push_back(NewGO<nsItem::CItem>(enPriority_Zeroth));
				m_item[m_itemNum]->SetPosition(objData.position);
				m_itemNum++;

				return true;
			}

			if (objData.EqualObjectName("torch")) {

				//燃えるエフェクトの座標と回転を調整。
				CVector3 effectPos = objData.position;
				effectPos += c_addFireEffectPosition;
				CQuaternion effectRot = objData.rotation;
				effectRot.AddRotationY(CMath::DegToRad(90.0f));
				effectPos = effectPos - objData.position;
				effectRot.Apply(effectPos);
				effectPos = objData.position + effectPos;

				//エフェクトを初期化。
				m_fireEffect.push_back(NewGO<Effect>(enPriority_Zeroth));
				m_fireEffect[m_fireEffectNum]->Init(c_filePathFireEffect);
				m_fireEffect[m_fireEffectNum]->SetScale(c_fireEffectScale);
				m_fireEffect[m_fireEffectNum]->SetPosition(effectPos);
				m_fireEffect[m_fireEffectNum]->SetRotation(effectRot);

				//炎エフェクトに対応するポイントライトを作成。
				m_pointLight.push_back(NewGO<nsLight::CPointLight>(enPriority_Zeroth));
				m_pointLight[m_pointLightNum]->SetPosition(effectPos);
				m_pointLight[m_pointLightNum]->SetColor(c_firePointLightColor);
				m_pointLight[m_pointLightNum]->SetRange(c_firePointLightRange);
				m_pointLight[m_pointLightNum]->SetAffectPowParam(c_firePointLightAffectParam);
				m_pointLightNum++;

				//再生。
				m_fireEffect[m_fireEffectNum]->Play();
				m_fireEffectNum++;
			}
			if (objData.EqualObjectName("torchBowl")) {

				//燃えるエフェクトの座標と回転を調整。
				CVector3 effectPos = objData.position;
				effectPos.y += 145.0f;

				//エフェクトを初期化。
				m_fireEffect.push_back(NewGO<Effect>(enPriority_Zeroth));
				m_fireEffect[m_fireEffectNum]->Init(u"Assets/effect/fire.efk");
				m_fireEffect[m_fireEffectNum]->SetScale({ 10.0f,10.0f,10.0f });
				m_fireEffect[m_fireEffectNum]->SetPosition(effectPos);

				//炎エフェクトに対応するポイントライトを作成。
				m_pointLight.push_back(NewGO<nsLight::CPointLight>(enPriority_Zeroth));
				m_pointLight[m_pointLightNum]->SetPosition(effectPos);
				m_pointLight[m_pointLightNum]->SetColor({ 2.0f,1.0f,1.0f });
				m_pointLight[m_pointLightNum]->SetRange(300.0f);
				m_pointLight[m_pointLightNum]->SetAffectPowParam(1.5f);
				m_pointLightNum++;

				//再生。
				m_fireEffect[m_fireEffectNum]->Play();
				m_fireEffectNum++;
			}
			return false;
			});
	}
}