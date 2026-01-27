import {
  Entity,
  Column,
  PrimaryGeneratedColumn,
  CreateDateColumn,
  UpdateDateColumn,
  DeleteDateColumn,
  OneToMany,
  Index,
} from 'typeorm';
import { Exclude } from 'class-transformer';
import { UserRole, UserStatus } from '@enums/user.enum';

@Entity('users')
@Index(['cinNumber'], {
  unique: true,
  where: 'cin IS NOT NULL AND deleted_at IS NULL',
})
@Index(['studentId'], {
  unique: true,
  where: 'student_id IS NOT NULL AND deleted_at IS NULL',
})
export class User {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @CreateDateColumn()
  createdAt: Date;

  @UpdateDateColumn()
  updatedAt: Date;

  @DeleteDateColumn()
  deletedAt?: Date;

  @Column({ unique: true, type: 'varchar', length: 255 })
  email: string;

  @Column()
  @Exclude()
  password: string;

  @Column({ name: 'first_name' })
  firstName: string;

  @Column({ name: 'last_name' })
  lastName: string;

  @Column({ name: 'phone_number', nullable: true })
  phoneNumber?: string;

  @Column({
    type: 'enum',
    enum: UserRole,
    default: UserRole.STUDENT,
  })
  role: UserRole;

  @Column({
    type: 'enum',
    enum: UserStatus,
    default: UserStatus.INACTIVE,
  })
  status: UserStatus;

  @Column({ name: 'cin_number', nullable: true, unique: true, length: 12 })
  cinNumber?: string;
  @Column({
    name: 'student_id',
    type: 'varchar',
    length: 20,
    unique: true,
    nullable: true,
  })
  studentId?: string;
  @Column({ name: 'cin_pictures', nullable: true })
  studentCard?: string;

  @Column({ name: 'is_verified', default: false })
  isVerified: boolean;

  @Column({ name: 'verified_by', nullable: true })
  verifiedBy?: string;

  @Column({ name: 'verified_at', nullable: true })
  verifiedAt?: Date;

  getstudentCardArray(): string[] {
    if (!this.studentCard) return [];
    try {
      return JSON.parse(this.studentCard);
    } catch {
      return [];
    }
  }
  setstudentCardArray(paths: string[]): void {
    this.studentCard = JSON.stringify(paths);
  }
}
