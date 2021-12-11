#include "stdafx.h"
#include "PlayerTriggerBox.h"
#include "../enemy/Enemy.h"

namespace nsMyGame {

	namespace nsPlayer {

		void CPlayerTriggerBox::Update() {

			if (!m_isActive) { return; }

			//名前がEnemyのオブジェクトをCEnemyクラスにキャスト。
			QueryGOs<nsEnemy::CEnemy>(c_classNameEnemy, [this](nsEnemy::CEnemy* enemy) {

				//剛体との当たり判定を調べる。
				CPhysicsWorld::GetInstance()->ContactTest(enemy->GetCharacterController(), [&](const btCollisionObject& contactObject) {

					//まだ敵が今回の攻撃を受けていない状態でトリガーボックスと接触した。
					if (m_attackCollision.IsSelf(contactObject)) {

						//敵にダメージを与える。
						enemy->SetReceiveDamage(true);
					}
				});
				return true;
			});
		}
	}
}