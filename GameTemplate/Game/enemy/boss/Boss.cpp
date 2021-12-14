#include "stdafx.h"
#include "Boss.h"
#include "../../AttackCollision.h"

namespace nsMyGame {

	namespace nsEnemy {

		extern CEnemy* g_pCurrentEnemy;

		bool CBoss::StartSub() {

			//IGameObjectに追加。
			m_modelRender = NewGO<CModelRender>(enPriority_Zeroth);

			//tkmファイルとtksファイルを設定。
			m_modelRender->SetFilePathTkm(c_filePathTkmBoss);
			m_modelRender->SetFilePathTks(c_filePathTksBoss);

			//座標を設定。
			m_modelRender->SetPosition(m_position);

			//回転を設定。
			m_modelRender->SetRotation(m_rotation);

			//アニメーションクリップを初期化。
			InitAnimationClip();

			//アニメーションを初期化。
			m_modelRender->InitAnimation(m_animationClip, enAnim_Num);

			//影を落とす。
			m_modelRender->SetShadowCasterFlag(true);

			//影を受ける。
			m_modelRender->SetShadowReceiverFlag(true);

			//ステータスを初期化。
			InitStatus();

			//初期化。
			m_modelRender->Init();

			//アニメーションイベント用の関数を設定する。
			m_modelRender->AddAnimationEvent([&](const wchar_t* clipName, const wchar_t* eventName) {

				OnAnimationEvent(clipName, eventName);
			});

			//キャラクターコントローラーを初期化。
			m_charaCon.Init(
				20.0f,			//半径。
				100.0f,			//高さ。
				m_position		//座標。
			);

			//剣に取り付けられたボーンの番号を読み込む。
			auto swordBoneNum = m_modelRender->GetSkeleton()->FindBoneID(L"mixamorig5:LeftHand");

			//剣のボーンを取得。
			m_swordBone = m_modelRender->GetSkeleton()->GetBone(swordBoneNum);

			//剣のボーンのワールド行列を取得。
			CMatrix swordBaseMatrix = m_swordBone->GetWorldMatrix();

			//当たり判定のインスタンスを初期化。
			m_triggerBox.Create(m_position, m_rotation);

			//当たり判定の座標と回転を更新。
			m_triggerBox.UpdatePositionAndRotation(swordBaseMatrix);

			//当たり判定をしないように設定。
			m_triggerBox.Deactivate();

			return true;
		}

		void CBoss::UpdateSub() {

			// 現在更新処理を実行中のエネミーのアドレスを代入
			g_pCurrentEnemy = this;

			//剣のボーンのワールド行列を取得。
			CMatrix swordBaseMatrix = m_swordBone->GetWorldMatrix();

			//当たり判定のワールド行列を更新。
			m_triggerBox.UpdatePositionAndRotation(swordBaseMatrix);

			//ステートに応じて読み込むPythonスクリプトを変える。
			switch (m_state) {
			case enState_Idle:
				ImportModule("BossIdle");
				break;
			case enState_Walk:
				ImportModule("BossMove");
				break;
			case enState_JumpAttack:
				ImportModule("BossJumpAttack");
				break;
			case enState_SwipingAttack:
				ImportModule("BossSwipingAttack");
				break;
			case enState_Damage:
				ImportModule("EnemyDamage");
				break;
			case enState_Death:
				ImportModule("EnemyDeath");
				break;
			case enState_AttackBreak:
				ImportModule("EnemyAttackBreak");
				break;
			}

			//PythonスクリプトのUpdate()関数を呼び出す。
			auto updateFunc = m_enemyPyModule.attr("Update");
			updateFunc();
		}

		void CBoss::InitStatus() {

			m_status.hp = 100;
			m_status.attack = 10;
		}

		void CBoss::InitAnimationClip() {

			//アニメーションクリップを設定。
			m_animationClip[enAnim_Walk].Load("Assets/animData/Boss/walk.tka");
			m_animationClip[enAnim_Walk].SetLoopFlag(true);
			m_animationClip[enAnim_Idle].Load("Assets/animData/Boss/idle.tka");
			m_animationClip[enAnim_Idle].SetLoopFlag(true);
			m_animationClip[enAnim_JumpAttack].Load("Assets/animData/Boss/attack.tka");
			m_animationClip[enAnim_JumpAttack].SetLoopFlag(false);
			m_animationClip[enAnim_SwipingAttack].Load("Assets/animData/Boss/swiping.tka");
			m_animationClip[enAnim_SwipingAttack].SetLoopFlag(false);
			m_animationClip[enAnim_Damage].Load("Assets/animData/damage.tka");
			m_animationClip[enAnim_Damage].SetLoopFlag(false);
			m_animationClip[enAnim_Death].Load("Assets/animData/death2.tka");
			m_animationClip[enAnim_Death].SetLoopFlag(false);
			m_animationClip[enAnim_AttackBreak].Load("Assets/animData/attackBreak.tka");
			m_animationClip[enAnim_AttackBreak].SetLoopFlag(false);
		}

		void CBoss::AnimationUpdate() {

			//各ステートに対応したアニメーションを再生する。
			switch (m_state) {
			case enState_Idle:
				m_modelRender->PlayAnimation(enAnim_Idle, 0.4f);
				break;
			case enState_Walk:
				m_modelRender->PlayAnimation(enAnim_Walk, 0.4f);
				break;
			case enState_JumpAttack:
				m_modelRender->PlayAnimation(enAnim_JumpAttack, 0.4f);
				break;
			case enState_SwipingAttack:
				m_modelRender->PlayAnimation(enAnim_SwipingAttack, 0.4f);
				break;
			case enState_Damage:
				m_modelRender->PlayAnimation(enAnim_Damage, 0.4f);
				break;
			case enState_Death:
				m_modelRender->PlayAnimation(enAnim_Death, 0.4f);
				break;
			case enState_AttackBreak:
				m_modelRender->PlayAnimation(enAnim_AttackBreak, 0.4f);
				break;
			}
		}

		void CBoss::OnAnimationEvent(const wchar_t* clipName, const wchar_t* eventName)
		{
			//キーの名前が「attack」の時。
			if (wcscmp(eventName, L"attack") == 0)
			{
				//攻撃中にする。
				m_triggerBox.Activate();
			}
			//キーの名前が「attack_end」の時。
			else if (wcscmp(eventName, L"attackEnd") == 0)
			{
				//攻撃を終わる。
				m_triggerBox.Deactivate();
			}
		}

		void CBoss::CreateAttackCollision() {

			//剣のボーンのワールド行列を取得。
			CMatrix swordBaseMatrix = m_swordBone->GetWorldMatrix();

			//コリジョンオブジェクトを作成。
			auto collisionObject = NewGO<CAttackCollision>(enPriority_Zeroth, c_classnameEnemyAttackCollision);

			//有効時間を設定。
			collisionObject->SetActiveTime(c_attackCollisionActiveTime);

			//ボックス状のコリジョンを作成。
			collisionObject->CreateBox(m_position, CQuaternion::Identity, c_attackTriggerBoxSize);

			//剣のボーンのワールド行列をコリジョンに適用。
			collisionObject->SetWorldMatrix(swordBaseMatrix);
		}

		void CBoss::Move() {

			//x方向とz方向の移動速度を初期化。
			m_moveSpeed.x = 0.0f;
			m_moveSpeed.z = 0.0f;

			//プレイヤーの座標を取得する。
			CVector3 toPlayerVec = m_player->GetPosition() - m_position;
			//正規化。
			toPlayerVec.Normalize();

			//歩き状態ならプレイヤーに一定速度で近づく。
			if (m_state == enState_Walk) {

				m_moveSpeed += toPlayerVec * 240.0f;
			}

			//3連続攻撃状態なら一定速度でプレイヤーに近づく。
			else if (m_state == enState_JumpAttack) {

				if (c_threeComboCoolTime - m_coolTime < 2.4f) {
					m_moveSpeed += toPlayerVec * 50.0f;
				}
			}

			//重力をかける。
			m_moveSpeed.y -= 980.0f * g_gameTime->GetFrameDeltaTime();

			//座標を設定。
			m_position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

			//地面についているか判定。
			if (m_charaCon.IsOnGround()) {

				//地面についているなら下向きには力をかけない。
				m_moveSpeed.y = 0.0f;
			}
		}

		void CBoss::UpdateTriggerBox() {

			//トリガーボックスを更新。
			m_triggerBox.Update();
		}
	}
}